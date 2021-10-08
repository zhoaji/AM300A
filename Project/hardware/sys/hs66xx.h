
#ifndef __HS66XX_H
#define __HS66XX_H

#include "features.h"

#if defined(CONFIG_HS6621)
#include "hs6621.h"
#elif defined(CONFIG_HS6621C)
#include "hs6621c.h"
#elif defined(CONFIG_HS6621P)
#include "hs6621p.h"
#endif

#endif
