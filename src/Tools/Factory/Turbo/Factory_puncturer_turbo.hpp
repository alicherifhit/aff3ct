#ifndef FACTORY_PUNCTURER_TURBO_HPP
#define FACTORY_PUNCTURER_TURBO_HPP

#include "Module/Puncturer/Puncturer.hpp"
#include "Tools/params.h"
#include "Tools/Code/Polar/Frozenbits_generator/Frozenbits_generator.hpp"

#include "../Factory.hpp"

namespace aff3ct
{
namespace tools
{
template <typename B = int, typename Q = float>
struct Factory_puncturer_turbo : public Factory
{
	static module::Puncturer<B,Q>* build(const parameters &params);
};
}
}

#endif /* FACTORY_PUNCTURER_TURBO_HPP */