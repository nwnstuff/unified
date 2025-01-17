#pragma once
#include "nwn_api.hpp"



#ifdef NWN_API_PROLOGUE
NWN_API_PROLOGUE(ENCAPSULATED_KEYLISTENTRY)
#endif



typedef uint16_t RESTYPE;
typedef uint8_t RESREF[16];


struct ENCAPSULATED_KEYLISTENTRY
{
    RESREF resRef;
    uint32_t nID;
    RESTYPE nType;
    uint16_t unused;



#ifdef NWN_CLASS_EXTENSION_ENCAPSULATED_KEYLISTENTRY
    NWN_CLASS_EXTENSION_ENCAPSULATED_KEYLISTENTRY
#endif
};


#ifdef NWN_API_EPILOGUE
NWN_API_EPILOGUE(ENCAPSULATED_KEYLISTENTRY)
#endif

