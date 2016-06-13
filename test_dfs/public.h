/*
 * public.h
 *
 *  Created on: May 31, 2016
 *      Author: zpzhao
 */

#ifndef PUBLIC_H_
#define PUBLIC_H_



#define LOG_POSITION(format, ...) printf(format,##__VA_ARGS__);
#define LOG(format, ...) LOG_POSITION("[%s][%d]  "format, __FUNCTION__, __LINE__, ##__VA_ARGS__);




#endif /* PUBLIC_H_ */
