#include <Arduino.h>
#include <stdlib.h>
#include "digitalbutton.h"

typedef struct DigitalButtonState {
    int pin;
    int holdMin;
    int debounceDown;
    int debounceUp;
    int debounceTap;
    buttonCallback * onDown;
    buttonCallback * onUp;
    buttonCallback * onTap;
    buttonCallback * onHold;

    // if button is currently down, this was
    // when the down press began
    unsigned long currentDown;
    bool isHold;
    unsigned long lastDown;
    unsigned long lastUp;
    unsigned long lastTap;
} DigitalButtonState;

static void triggerDown(DigitalButtonState * state){
    if(state->onDown != NULL){
        state->onDown();
    }
}
static void triggerUp(DigitalButtonState * state){
    if(state->onUp != NULL){
        state->onUp();
    }
}
static void triggerTap(DigitalButtonState * state){
    if(state->onTap != NULL){
        state->onTap();
    }
}
static void triggerHold(DigitalButtonState * state){
    if(state->onHold != NULL){
        state->onHold();
    }
}

void digitalButtonTick(DigitalButton s){
    DigitalButtonState * state = (DigitalButtonState*)s;
    int buttonPosition = digitalRead(state->pin);
    unsigned long now = millis();

    // button is currently pressed
    // TODO - this assumes were using pullup
    if(buttonPosition == LOW){

        // button has just been pressed
        if(!state->currentDown){
            state->currentDown = now;
            state->lastDown = now;

        // button has continued to be pressed
        } else {
            // if this has not become a hold event yet,
            // and the elapsed hold time meets the
            // hold min, make this a hold event
            if(!state->isHold && state->holdMin &&
                    now - state->currentDown >= state->holdMin){
                triggerHold(state);
                state->isHold = true;
            }
        }

    // button aint pressed right now guys.
    } else if(buttonPosition == HIGH){

        // button was just released
        if(state->currentDown){
            state->lastUp = now;
            //triggerUp(state);

            // if this wasnt a hold, it's a tap
            if(!state->isHold){
                //triggerTap(state);
                state->lastTap = now;
            }

            // clear event-specific state
            state->currentDown = 0;
            state->isHold = false;

        // button remains unprexed
        } else {
            // :(
        }
    }

    // determine if any event handlers should be called
    if(state->lastDown != 0 && now - state->lastDown > state->debounceDown){
        state->lastDown = 0;
        triggerDown(state);
    }
    if(state->lastUp != 0 && now - state->lastUp > state->debounceUp){
        state->lastUp = 0;
        triggerUp(state);
    }
    if(state->lastTap != 0 && now - state->lastTap > state->debounceTap){
        state->lastTap = 0;
        triggerTap(state);
    }
}

DigitalButton buttonCreate(int pin){
    DigitalButtonState * state = (DigitalButtonState*)calloc(1, sizeof(DigitalButtonState)); 
    state->pin = pin;

    // TODO - configurable pinMode?
    pinMode(pin, INPUT_PULLUP);
    digitalWrite(pin, HIGH);

    // loopAttach(digitalButtonTick, debounceTime, state);
    return state;
}

void buttonOnDown(DigitalButton state, buttonCallback cb, int debounceTime){
    state->onDown = cb;
    state->debounceDown = debounceTime;
}
void buttonOnUp(DigitalButton state, buttonCallback cb, int debounceTime){
    state->onUp = cb;
    state->debounceUp = debounceTime;
}
void buttonOnTap(DigitalButton state, buttonCallback cb, int debounceTime){
    state->onTap = cb;
    state->debounceTap = debounceTime;
}
void buttonOnHold(DigitalButton state, buttonCallback cb, int holdMin){
    state->onHold = cb;
    state->holdMin = holdMin;
}
