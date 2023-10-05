/*
 * =====================================================================================
 *
 *       Filename:  singleton.h
 *
 *    Description:  单例
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:
 *
 *         Author:
 *        Company:
 *
 * =====================================================================================
 */

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

namespace base {
// 单例模式宏
// 放在类的定义中
#define SINGLETON_PATTERN_DECLARE(typename)                                    \
public:                                                                        \
  static typename &getInstance();                                              \
  ~typename();                                                                 \
                                                                               \
private:                                                                       \
  typename();

// 放在类的实现中
#define SINGLETON_PATTERN_IMPLEMENT(typename)                                  \
  typename &typename ::getInstance() {                                         \
    static typename instance##classname;                                       \
    return instance##classname;                                                \
  }
} // namespace base
#endif