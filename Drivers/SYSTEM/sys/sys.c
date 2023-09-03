/**
 ****************************************************************************************************
 * @file        sys.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-17
 * @brief       ϵͳ��ʼ������(����ʱ������/�жϹ���/GPIO���õ�)
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20211103
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "sys.h"

/**
 * @brief       �����ж�������ƫ�Ƶ�ַ
 * @param       baseaddr: ��ַ
 * @param       offset: ƫ����(������0, ����0X100�ı���)
 * @retval      ��
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset) {
	/* ����NVIC��������ƫ�ƼĴ���,VTOR��9λ����,��[8:0]���� */
	SCB->VTOR = baseaddr | (offset & (uint32_t) 0xFFFFFE00);
}

/**
 * @brief       ִ��: WFIָ��(ִ�����ָ�����͹���״̬, �ȴ��жϻ���)
 * @param       ��
 * @retval      ��
 */
void sys_wfi_set(void) {
	__ASM volatile("wfi");
}

/**
 * @brief       �ر������ж�(���ǲ�����fault��NMI�ж�)
 * @param       ��
 * @retval      ��
 */
void sys_intx_disable(void) {
	__ASM volatile("cpsid i");
}

/**
 * @brief       ���������ж�
 * @param       ��
 * @retval      ��
 */
void sys_intx_enable(void) {
	__ASM volatile("cpsie i");
}

/**
 * @brief       ����ջ����ַ
 * @note        ���ĺ�X, ����MDK��, ʵ����û�����
 * @param       addr: ջ����ַ
 * @retval      ��
 */
void sys_msr_msp(uint32_t addr) {
	__set_MSP(addr); /* ����ջ����ַ */
}

/**
 * @brief       �������ģʽ
 * @param       ��
 * @retval      ��
 */
void sys_standby(void) {
	__HAL_RCC_PWR_CLK_ENABLE(); /* ʹ�ܵ�Դʱ�� */
	SET_BIT(PWR->CR, PWR_CR_PDDS); /* �������ģʽ */
}

/**
 * @brief       ϵͳ��λ
 * @param       ��
 * @retval      ��
 */
void sys_soft_reset(void) {
	NVIC_SystemReset();
}

/**
 * @brief       ϵͳʱ�ӳ�ʼ������
 * @param       plln: PLL��Ƶϵ��(PLL��Ƶ), ȡֵ��Χ: 2~16
 �ж�������λ��������ʱ�Ѿ���SystemInit()�г�ʼ��
 * @retval      ��
 */
void sys_stm32_clock_init(uint32_t plln) {
	HAL_StatusTypeDef ret = HAL_ERROR;
	RCC_OscInitTypeDef rcc_osc_init = { 0 };
	RCC_ClkInitTypeDef rcc_clk_init = { 0 };

	rcc_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE; /* ѡ��Ҫ����HSE */
	rcc_osc_init.HSEState = RCC_HSE_ON; /* ��HSE */
	rcc_osc_init.HSEPredivValue = RCC_HSE_PREDIV_DIV1; /* HSEԤ��Ƶϵ�� */
	rcc_osc_init.PLL.PLLState = RCC_PLL_ON; /* ��PLL */
	rcc_osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE; /* PLLʱ��Դѡ��HSE */
	rcc_osc_init.PLL.PLLMUL = plln; /* PLL��Ƶϵ�� */
	ret = HAL_RCC_OscConfig(&rcc_osc_init); /* ��ʼ�� */

	if (ret != HAL_OK) {
		while (1)
			; /* ʱ�ӳ�ʼ��ʧ�ܣ�֮��ĳ��򽫿����޷�����ִ�У���������������Լ��Ĵ��� */
	}

	/* ѡ��PLL��Ϊϵͳʱ��Դ��������HCLK,PCLK1��PCLK2*/
	rcc_clk_init.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	rcc_clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* ����ϵͳʱ������PLL */
	rcc_clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1; /* AHB��Ƶϵ��Ϊ1 */
	rcc_clk_init.APB1CLKDivider = RCC_HCLK_DIV2; /* APB1��Ƶϵ��Ϊ2 */
	rcc_clk_init.APB2CLKDivider = RCC_HCLK_DIV1; /* APB2��Ƶϵ��Ϊ1 */
	ret = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_2); /* ͬʱ����FLASH��ʱ����Ϊ2WS��Ҳ����3��CPU���ڡ� */

	if (ret != HAL_OK) {
		while (1)
			; /* ʱ�ӳ�ʼ��ʧ�ܣ�֮��ĳ��򽫿����޷�����ִ�У���������������Լ��Ĵ��� */
	}
}

