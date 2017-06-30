#ifndef FACTORY_DECODER_LDPC_HPP
#define FACTORY_DECODER_LDPC_HPP

#include <string>
#include <mipp.h>

#include "Tools/Algo/Sparse_matrix/Sparse_matrix.hpp"

#include "Module/Decoder/Decoder.hpp"
#include "Module/Decoder/Decoder_SISO.hpp"

#include "../Factory_decoder_common.hpp"

namespace aff3ct
{
namespace tools
{
template <typename B = int, typename R = float>
struct Factory_decoder_LDPC : public Factory_decoder_common
{
	struct decoder_parameters_LDPC : decoder_parameters
	{
		virtual ~decoder_parameters_LDPC() {}

		// ------- decoder
		std::string simd_strategy    = "";
		float       norm_factor      = 1.f;
		float       offset           = 0.f;
		bool        enable_syndrome  = true;
		int         syndrome_depth   = 2;
		int         n_ite            = 10;

		// ------- code
		std::string alist_path = "";
	};

	static module::Decoder_SISO<B,R>* build(const decoder_parameters_LDPC& params, const Sparse_matrix &H,
	                                        const std::vector<unsigned> &info_bits_pos);

	static void build_args(Arguments_reader::arg_map &req_args, Arguments_reader::arg_map &opt_args);
	static void store_args(const Arguments_reader& ar, decoder_parameters_LDPC &params, int K, int N, int n_frames = 1);
	static void group_args(Arguments_reader::arg_grp& ar);

	static void header(Header::params_list& head_dec, Header::params_list& head_cde, const decoder_parameters_LDPC& params);
};
}
}

#endif /* FACTORY_DECODER_LDPC_HPP */
