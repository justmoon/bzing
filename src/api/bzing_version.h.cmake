#ifndef BZING_VERSION_H_
#define BZING_VERSION_H_

#include <bzing/bzing_common.h>

#define BZING_MAJOR ${BZING_MAJOR}
#define BZING_MINOR ${BZING_MINOR}
#define BZING_MICRO ${BZING_MICRO}

#define BZING_VERSION ((BZING_MAJOR * 10000) + (BZING_MINOR * 100) + BZING_MICRO)

#ifdef __cplusplus
extern "C" {
#endif

extern int BZING_API bzing_version(void);

#ifdef __cplusplus
}
#endif

#endif /* BZING_VERSION_H_ */
