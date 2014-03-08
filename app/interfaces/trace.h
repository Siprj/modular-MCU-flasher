#ifndef TRACE_H
#define TRACE_H


#ifdef TRACE_FUNCTION_CALLS
#include <QDebug>

#define FUNCTION_ENTER_TRACE qDebug()<<"ENTER";

#else

#define FUNCTION_ENTER_TRACE ;

#endif


#endif // TRACE_H
