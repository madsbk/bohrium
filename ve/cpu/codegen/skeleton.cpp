#include <sstream>
#include <string>
#include <map>
#include "codegen.hpp"

using namespace std;
using namespace kp::core;

namespace kp{
namespace engine{
namespace codegen{

Skeleton::Skeleton(Plaid& plaid, std::string skeleton) : plaid_(plaid), skeleton_(skeleton) {}

void Skeleton::reset(void)
{
    subjects_.clear();
}

string Skeleton::emit(void)
{
    return plaid_.fill(skeleton_, subjects_);
}

}}}
