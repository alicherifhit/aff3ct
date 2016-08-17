#include <limits>

#include "../../../Tools/bash_tools.h"
#include "../../decoder_functions.h"

#include "Decoder_LDPC_BP_min_sum_naive.hpp"

constexpr int C_to_V_max = 15; // saturation value for the LLRs/extrinsics

template <typename B, typename R>
Decoder_LDPC_BP_min_sum_naive<B,R>
::Decoder_LDPC_BP_min_sum_naive(const int &N, const int &K, const int& n_ite,
                                const std::vector<unsigned char> &n_variables_per_parity,
                                const std::vector<unsigned char> &n_parities_per_variable,
                                const std::vector<unsigned int > &transpose)
: Decoder_LDPC_BP_naive<B,R>(N, K, n_ite, n_variables_per_parity, n_parities_per_variable, transpose)
{
}

template <typename B, typename R>
Decoder_LDPC_BP_min_sum_naive<B,R>
::~Decoder_LDPC_BP_min_sum_naive()
{
}

// BP-like algorithm (normalized offest min-sum)
template <typename B, typename R>
bool Decoder_LDPC_BP_min_sum_naive<B,R>
::BP_decode(const mipp::vector<R> &Y_N)
{
	// actual decoding
	auto syndromes = 0;
	for (auto ite = 0; ite < this->n_ite; ite++)
	{
		syndromes = 0;

		// begining of the iteration upon all the matrix lines
		R *C_to_V_ptr = this->C_to_V.data();
		R *V_to_C_ptr = this->V_to_C.data();

		for (auto i = 0; i < this->n_V_nodes; i++)
		{
			// VN node accumulate all the incoming messages
			const auto length = this->n_parities_per_variable[i];

			auto sum_C_to_V = (R)0;
			for (auto j = 0; j < length; j++)
				sum_C_to_V += C_to_V_ptr[j];

			// update the intern values
			const auto temp = Y_N[i] + sum_C_to_V;

			// generate the outcoming messages to the CNs
			for (auto j = 0; j < length; j++)
				V_to_C_ptr[j] = temp - C_to_V_ptr[j];

			C_to_V_ptr += length; // jump to the next node
			V_to_C_ptr += length; // jump to the next node
		}
		
		auto transpose_ptr = this->transpose.data();
		for (auto i = 0; i < this->n_C_nodes; i++) 
		{
			const auto length = this->n_variables_per_parity[i];

			auto sign = 0;
			auto min1 = std::numeric_limits<R>::max();
			auto min2 = std::numeric_limits<R>::max();

			// accumulate the incoming information in CN
			for (auto j = 0; j < length; j++)
			{
				const auto value  = this->V_to_C[transpose_ptr[j]];
				const auto c_sign = std::signbit(value) ? -1 : 0;
				const auto v_abs  = (R)std::abs(value); // std::trunc(std::abs(value));
				const auto v_temp = min1;

				sign ^= c_sign;
				min1  = std::min(min1,          v_abs         ); // 1st min
				min2  = std::min(min2, std::max(v_abs, v_temp)); // 2nd min
			}

			// compute CN inter values
			constexpr float offset     = 0.000; // 0.00 // 0.15
			constexpr float normalize1 = 0.825; // 1.00 // 1.25 // 0.825
			constexpr float normalize2 = 0.825; // 1.00 // 1.25 // 0.825

			auto cste1 = (min2 - offset) * normalize1;
			auto cste2 = (min1 - offset) * normalize2;
			cste1 = (cste1 < 0) ? 0 : cste1;
			cste2 = (cste2 < 0) ? 0 : cste2;

			// regenerate the CN outcoming values
			for (auto j = 0; j < length; j++)
			{
				const auto value   = this->V_to_C[transpose_ptr[j]];
				const auto v_abs   = (R)std::abs(value);                    // std::trunc(std::abs(value));
				const auto v_res   = ((v_abs == min1) ? cste1 : cste2);     // cmov
				const auto v_sig   = sign ^ (std::signbit(value) ? -1 : 0); // xor bit
				const auto v_to_st = (R)std::copysign(v_res, v_sig);        // magnitude of v_res, sign of v_sig

				this->C_to_V[transpose_ptr[j]] = v_to_st;
			}

			transpose_ptr += length;
			syndromes = syndromes || sign;
		}

		// make a saturation
		saturate<R>(this->C_to_V, (R)-C_to_V_max, (R)C_to_V_max);

		// stop criterion
		if (syndromes == 0)
			break;
	}

	// begining of the iteration upon all the matrix lines
	R *C_to_V_ptr = this->C_to_V.data();
	for (auto i = 0; i < this->n_V_nodes; i++) 
	{
		const auto length = this->n_parities_per_variable[i];

		auto sum_C_to_V = (R)0;
		for (auto j = 0; j < length; j++)
			sum_C_to_V += C_to_V_ptr[j];

		// filling the output
		this->Lp_N[i] = Y_N[i] + sum_C_to_V;

		C_to_V_ptr += length;
	}

	return (syndromes == 0);
}

// ==================================================================================== explicit template instantiation 
#include "../../../Tools/types.h"
#ifdef MULTI_PREC
template class Decoder_LDPC_BP_min_sum_naive<B_8,Q_8>;
template class Decoder_LDPC_BP_min_sum_naive<B_16,Q_16>;
template class Decoder_LDPC_BP_min_sum_naive<B_32,Q_32>;
template class Decoder_LDPC_BP_min_sum_naive<B_64,Q_64>;
#else
template class Decoder_LDPC_BP_min_sum_naive<B,Q>;
#endif
// ==================================================================================== explicit template instantiation