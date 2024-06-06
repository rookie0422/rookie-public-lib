#include "systemstore.h"
#include <string.h>

systemstore_t g_systemstore;

/**
  * @brief  Program halfword, word or double word at a specified address
  * @note   The function HAL_FLASH_Unlock() should be called before to unlock the FLASH interface
  *         The function HAL_FLASH_Lock() should be called after to lock the FLASH interface
  *
  * @note   If an erase and a program operations are requested simultaneously,    
  *         the erase operation is performed before the program one.
  *  
  * @note   FLASH should be previously erased before new programming (only exception to this 
  *         is when 0x0000 is programmed)
  *
  * @param  TypeProgram   Indicate the way to program at a specified address.
  *                       This parameter can be a value of @ref FLASH_Type_Program
  * @param  Address       Specifie the address to be programmed.
  * @param  Data          Specifie the data to be programmed
  * 
  * @retval HAL_StatusTypeDef HAL Status
  */
HAL_StatusTypeDef HAL_FLASH_Program32(uint32_t Address, uint32_t Data)
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint8_t index = 0U;
    uint8_t nbiterations = 0U;

    /* Check the parameters */
    assert_param(IS_FLASH_TYPEPROGRAM(TypeProgram));
    assert_param(IS_FLASH_PROGRAM_ADDRESS(Address));

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    if (status == HAL_OK) {
        /* Proceed to program the new data */
        SET_BIT(FLASH->CR, FLASH_CR_PG);

        /* Write data in the address */
        *(__IO uint32_t *)Address = Data;

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

        /* If the program operation is completed, disable the PG Bit */
        CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
        /* In case of error, stop programming procedure */
        if (status != HAL_OK) {
            return status;
        }
    }

    return status;
}

/**
 * @brief 将系统配置保存到Flash
 *
 */
#if 1
void systemstore__save()
{
    uint32_t PageError = 0; //设置PageError,如果出现错误这个变量会被设置为出错的FLASH地址
    uint8_t i;
    uint32_t *pu32;

    FLASH_EraseInitTypeDef My_Flash; //声明 FLASH_EraseInitTypeDef 结构体为 My_Flash
    HAL_FLASH_Unlock();              //解锁Flash

    My_Flash.TypeErase = FLASH_TYPEERASE_PAGES; //标明Flash执行页面只做擦除操作
    My_Flash.PageAddress = DATASTORE_BASEADDR;  //声明要擦除的地址
    My_Flash.NbPages = 1; //说明要擦除的页数，此参数必须是Min_Data = 1和Max_Data =(最大页数-初始页的值)之间的值
    HAL_FLASHEx_Erase(&My_Flash, &PageError); //调用擦除函数擦除
                                              //	HAL_FLASH_Lock(); //锁住Flash

    pu32 = (uint32_t *)&g_systemstore;
    //	HAL_FLASH_Unlock();               //解锁Flash

    for (i = 0; i < sizeof(systemstore_t) / 4;
         i++) { //The sizeof systemstore is guaranteed as a multiple of 4, so directly divided by 4
        HAL_FLASH_Program32(
            DATASTORE_BASEADDR + i * 4,
            pu32[i]); //对Flash进行烧写，FLASH_TYPEPROGRAM_HALFWORD 声明操作的Flash地址的16位的，此外还有32位跟64位的操作，自行翻查HAL库的定义即可
    }

    HAL_FLASH_Lock(); //锁住Flash
}
#endif
#if 0
void systemstore__save()
{
	uint32_t PageError = 0;           //设置PageError,如果出现错误这个变量会被设置为出错的FLASH地址
	uint8_t i;
	uint32_t *pu32;

	fmc_unlock();
	flash_erase_page(DATASTORE_BASEADDR);

	pu32 = (uint32_t *)&g_systemstore;
	for (i=0; i<sizeof(systemstore_t)/4; i++) { //The sizeof systemstore is guaranteed as a multiple of 4, so directly divided by 4
		fmc_word_program(DATASTORE_BASEADDR + i*4, pu32[i]); //对Flash进行烧写，FLASH_TYPEPROGRAM_HALFWORD 声明操作的Flash地址的16位的，此外还有32位跟64位的操作，自行翻查HAL库的定义即可
	}

	fmc_lock(); //锁住Flash
}
#endif

/*!
*  \brief  判断flash内是否存有数据
*/
static uint8_t systemstore__is_init()
{
    if (g_systemstore.continuous_speed == 0xFFFF || g_systemstore.manual_forward_speed == 0xFFFF) {
        return 0;
    } else
        return 1;
}

/*!
*  \brief  默认配置（仅在flash内没有数据时使用）
*/
void systemstore_load_default_config()
{
    memset(&g_systemstore, 0, sizeof(systemstore_t));

    g_systemstore.continuous_speed = 300;   //连续模式速度
    g_systemstore.wire_back_length = 99;    //回抽长度
    g_systemstore.wire_forward_length = 99; //补丝长度

    g_systemstore.pulse_speed = 99;      //脉冲模式速度
    g_systemstore.pulse_period = 100;    //脉冲周期
    g_systemstore.pulse_smoothness = 25; //脉冲平滑度

    g_systemstore.manual_back_speed = 600; //手动回抽速度

    g_systemstore.manual_forward_speed = 600; //手动送丝速度`
}

/**
 * @brief 从Flash中读取系统配置
 *
 */
void systemstore__load()
{
    uint32_t *pu32;

    g_systemstore = *((systemstore_t *)DATASTORE_BASEADDR);

    if (systemstore__is_init() == 0) {
        //如果4个密码都是FF，说明配置为空，装入缺省配置
        systemstore_load_default_config();
    }
}
