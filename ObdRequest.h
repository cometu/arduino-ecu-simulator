#ifndef ObdRequest_h
#define ObdRequest_h

#include "Arduino.h"

typedef enum AnswerState
{
    IS_ANSWERED,
    ANSWER_IN_PROGRESS,
    NOT_ANSWERED
}AnswerState;

class ObdRequest
{
  public:
      bool isAnswered();
      
  private:
      AnswerState answerState;
      int maxStepToCompletion;
      int remainingStepToCompletion;
};

#endif
