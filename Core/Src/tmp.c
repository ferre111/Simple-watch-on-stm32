///*
// * tmp.c
// *
// *  Created on: Nov 22, 2020
// *      Author: Karol Witusik
// */
//

//HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
//{
//  __IO uint32_t count = 0U;
//
//  /* Init tickstart for timeout management*/
//  uint32_t tickstart = HAL_GetTick();
//
//  /* Check the parameters */
//  assert_param(IS_I2C_MEMADD_SIZE(MemAddSize));
//
//  if (hi2c->State == HAL_I2C_STATE_READY)
//  {
//    /* Wait until BUSY flag is reset */
//    if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG, tickstart) != HAL_OK)
//    {
//      return HAL_BUSY;
//    }
//
//    /* Process Locked */
//    __HAL_LOCK(hi2c);
//
//    /* Check if the I2C is already enabled */
//    if ((hi2c->Instance->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
//    {
//      /* Enable I2C peripheral */
//      __HAL_I2C_ENABLE(hi2c);
//    }
//
//    /* Disable Pos */
//    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_POS);
//
//    hi2c->State     = HAL_I2C_STATE_BUSY_RX;
//    hi2c->Mode      = HAL_I2C_MODE_MEM;
//    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
//
//    /* Prepare transfer parameters */
//    hi2c->pBuffPtr    = pData;
//    hi2c->XferCount   = Size;
//    hi2c->XferSize    = hi2c->XferCount;
//    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
//
//    /* Send Slave Address and Memory Address */
//    if (I2C_RequestMemoryRead(hi2c, DevAddress, MemAddress, MemAddSize, Timeout, tickstart) != HAL_OK)
//    {
//      return HAL_ERROR;
//    }
//
//    if (hi2c->XferSize == 0U)
//    {
//      /* Clear ADDR flag */
//      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
//
//      /* Generate Stop */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//    }
//    else if (hi2c->XferSize == 1U)
//    {
//      /* Disable Acknowledge */
//      CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
//
//      /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
//         software sequence must complete before the current byte end of transfer */
//      __disable_irq();
//
//      /* Clear ADDR flag */
//      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
//
//      /* Generate Stop */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//
//      /* Re-enable IRQs */
//      __enable_irq();
//    }
//    else if (hi2c->XferSize == 2U)
//    {
//      /* Enable Pos */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_POS);
//
//      /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
//         software sequence must complete before the current byte end of transfer */
//      __disable_irq();
//
//      /* Clear ADDR flag */
//      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
//
//      /* Disable Acknowledge */
//      CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
//
//      /* Re-enable IRQs */
//      __enable_irq();
//    }
//    else
//    {
//      /* Enable Acknowledge */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
//      /* Clear ADDR flag */
//      __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
//    }
//
//    while (hi2c->XferSize > 0U)
//    {
//      if (hi2c->XferSize <= 3U)
//      {
//        /* One byte */
//        if (hi2c->XferSize == 1U)
//        {
//          /* Wait until RXNE flag is set */
//          if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
//          {
//            return HAL_ERROR;
//          }
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//        }
//        /* Two bytes */
//        else if (hi2c->XferSize == 2U)
//        {
//          /* Wait until BTF flag is set */
//          if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, Timeout, tickstart) != HAL_OK)
//          {
//            return HAL_ERROR;
//          }
//
//          /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
//             software sequence must complete before the current byte end of transfer */
//          __disable_irq();
//
//          /* Generate Stop */
//          SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//
//          /* Re-enable IRQs */
//          __enable_irq();
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//        }
//        /* 3 Last bytes */
//        else
//        {
//          /* Wait until BTF flag is set */
//          if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BTF, RESET, Timeout, tickstart) != HAL_OK)
//          {
//            return HAL_ERROR;
//          }
//
//          /* Disable Acknowledge */
//          CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
//
//          /* Disable all active IRQs around ADDR clearing and STOP programming because the EV6_3
//             software sequence must complete before the current byte end of transfer */
//          __disable_irq();
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//
//          /* Wait until BTF flag is set */
//          count = I2C_TIMEOUT_FLAG * (SystemCoreClock / 25U / 1000U);
//          do
//          {
//            count--;
//            if (count == 0U)
//            {
//              hi2c->PreviousState       = I2C_STATE_NONE;
//              hi2c->State               = HAL_I2C_STATE_READY;
//              hi2c->Mode                = HAL_I2C_MODE_NONE;
//              hi2c->ErrorCode           |= HAL_I2C_ERROR_TIMEOUT;
//
//              /* Re-enable IRQs */
//              __enable_irq();
//
//              /* Process Unlocked */
//              __HAL_UNLOCK(hi2c);
//
//              return HAL_ERROR;
//            }
//          }
//          while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BTF) == RESET);
//
//          /* Generate Stop */
//          SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//
//          /* Re-enable IRQs */
//          __enable_irq();
//
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//        }
//      }
//      else
//      {
//        /* Wait until RXNE flag is set */
//        if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
//        {
//          return HAL_ERROR;
//        }
//
//        /* Read data from DR */
//        *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//        /* Increment Buffer pointer */
//        hi2c->pBuffPtr++;
//
//        /* Update counter */
//        hi2c->XferSize--;
//        hi2c->XferCount--;
//
//        if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BTF) == SET)
//        {
//          /* Read data from DR */
//          *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->DR;
//
//          /* Increment Buffer pointer */
//          hi2c->pBuffPtr++;
//
//          /* Update counter */
//          hi2c->XferSize--;
//          hi2c->XferCount--;
//        }
//      }
//    }
//
//    hi2c->State = HAL_I2C_STATE_READY;
//    hi2c->Mode = HAL_I2C_MODE_NONE;
//
//    /* Process Unlocked */
//    __HAL_UNLOCK(hi2c);
//
//    return HAL_OK;
//  }
//  else
//  {
//    return HAL_BUSY;
//  }
//}

///**
//  * @brief  Master sends target device address followed by internal memory address for read request.
//  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
//  *         the configuration information for I2C module
//  * @param  DevAddress Target device address: The device 7 bits address value
//  *         in datasheet must be shifted to the left before calling the interface
//  * @param  MemAddress Internal memory address
//  * @param  MemAddSize Size of internal memory address
//  * @param  Timeout Timeout duration
//  * @param  Tickstart Tick start value
//  * @retval HAL status
//  */
//static HAL_StatusTypeDef I2C_RequestMemoryRead(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout, uint32_t Tickstart)
//{
//  /* Enable Acknowledge */
//  SET_BIT(hi2c->Instance->CR1, I2C_CR1_ACK);
//
//  /* Generate Start */
//  SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
//
//  /* Wait until SB flag is set */
//  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
//  {
//    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
//    {
//      hi2c->ErrorCode = HAL_I2C_WRONG_START;
//    }
//    return HAL_TIMEOUT;
//  }
//
//  /* Send slave address */
//  hi2c->Instance->DR = I2C_7BIT_ADD_WRITE(DevAddress);
//
//  /* Wait until ADDR flag is set */
//  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
//  {
//    return HAL_ERROR;
//  }
//
//  /* Clear ADDR flag */
//  __HAL_I2C_CLEAR_ADDRFLAG(hi2c);
//
//  /* Wait until TXE flag is set */
//  if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
//  {
//    if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
//    {
//      /* Generate Stop */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//    }
//    return HAL_ERROR;
//  }
//
//  /* If Memory address size is 8Bit */
//  if (MemAddSize == I2C_MEMADD_SIZE_8BIT)
//  {
//    /* Send Memory Address */
//    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
//  }
//  /* If Memory address size is 16Bit */
//  else
//  {
//    /* Send MSB of Memory Address */
//    hi2c->Instance->DR = I2C_MEM_ADD_MSB(MemAddress);
//
//    /* Wait until TXE flag is set */
//    if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
//    {
//      if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
//      {
//        /* Generate Stop */
//        SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//      }
//      return HAL_ERROR;
//    }
//
//    /* Send LSB of Memory Address */
//    hi2c->Instance->DR = I2C_MEM_ADD_LSB(MemAddress);
//  }
//
//  /* Wait until TXE flag is set */
//  if (I2C_WaitOnTXEFlagUntilTimeout(hi2c, Timeout, Tickstart) != HAL_OK)
//  {
//    if (hi2c->ErrorCode == HAL_I2C_ERROR_AF)
//    {
//      /* Generate Stop */
//      SET_BIT(hi2c->Instance->CR1, I2C_CR1_STOP);
//    }
//    return HAL_ERROR;
//  }
//
//  /* Generate Restart */
//  SET_BIT(hi2c->Instance->CR1, I2C_CR1_START);
//
//  /* Wait until SB flag is set */
//  if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_SB, RESET, Timeout, Tickstart) != HAL_OK)
//  {
//    if (READ_BIT(hi2c->Instance->CR1, I2C_CR1_START) == I2C_CR1_START)
//    {
//      hi2c->ErrorCode = HAL_I2C_WRONG_START;
//    }
//    return HAL_TIMEOUT;
//  }
//
//  /* Send slave address */
//  hi2c->Instance->DR = I2C_7BIT_ADD_READ(DevAddress);
//
//  /* Wait until ADDR flag is set */
//  if (I2C_WaitOnMasterAddressFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, Timeout, Tickstart) != HAL_OK)
//  {
//    return HAL_ERROR;
//  }
//
//  return HAL_OK;
//}
