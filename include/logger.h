// Copyright (C) Alin Ichim 2023
#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

/**
 * @brief initializes logger.
 * 
 * @param filename Output filename.
 */
void logger_init(std::string filename);

/**
 * @brief Outputs informative message.
 * 
 * @param message The message.
 */
void logger_info(std::string message);

/**
 * @brief Outputs error message.
 * 
 * @param message The message.
 */
void logger_error(std::string message);

/**
 * @brief Outputs success message.
 * 
 * @param message The message.
 */
void logger_success(std::string message);

#endif  // LOGGER_H_