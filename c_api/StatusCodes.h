/************************************************************************************************************
 *                                          GGPO4ALL v0.0.1
 *              Created by Ranyodh Mandur - ✨ 2025 and GroundStorm Studios, LLC. - ✨ 2009
 *
 *                                Licensed under the MIT License (MIT).
 *                           For more details, see the LICENSE file or visit:
 *                                  https://opensource.org/licenses/MIT
 *
 *                        GGPO4ALL is a free open source rollback netcode library
************************************************************************************************************/
#ifndef GGPO4ALL_STATUS_CODES_C_H
#define GGPO4ALL_STATUS_CODES_C_H

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum GGPO4ALL_STATUS_CODE //only C23 supports the : int64_t definition which is not great for compatibility
    {
        //////////////////////// GGPO General ////////////////////////

        GGPO_OK = 1,

        GGPO_ERROR_INTERNAL_API_FAILURE,
        GGPO_ERROR_NULLPTR_REF_PASSED,
        GGPO_INVALID_SESSION_ID

    } StatusCodes;

#ifdef __cplusplus
}
#endif

#endif /* GGPO4ALL_STATUS_CODES_C_H */