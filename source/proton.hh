#ifndef PROTON_X_X_X__API
#define PROTON_X_X_X__API

#include "neutron.hh" // INJECT

namespace proton_X_X_X {

class Document
{
	virtual ::neutron_X_X_X::MemoryStream serialize() = 0;
	virtual void deserialize( ::neutron_X_X_X::MemoryStream & ) = 0;
};

} // namespace neutron_X_X_X

#endif // PROTON_X_X_X__API