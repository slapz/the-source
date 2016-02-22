/*
==============================================================================

Envelope.cpp
Created: 11 Dec 2015 3:14:14pm
Author:  Nhat Duc Tran and Anton Schmied

==============================================================================
*/

#include "Envelope.h"

void Envelope::resetReleaseCounter()
{
    releaseCounter = 0;
}

void Envelope::startEnvelope()
{
    releaseCounter = -1;
    attackDecayCounter = 0;
}

const float Envelope::calcEnvCoeff(float modValue1, float modValue2, bool isUnipolar1, bool isUnipolar2)
{
    // speed mod calculation
    attackSamples = calcModRange(modValue1, env.speedModAmount1.get(), static_cast<int>(sampleRate * env.attack.get()), isUnipolar1);
    attackSamples = calcModRange(modValue2, env.speedModAmount2.get(), attackSamples, isUnipolar2);

    decaySamples = calcModRange(modValue1, env.speedModAmount1.get(), static_cast<int>(sampleRate * env.decay.get()), isUnipolar1);
    decaySamples = calcModRange(modValue2, env.speedModAmount1.get(), decaySamples, isUnipolar2);

    releaseSamples = calcModRange(modValue1, env.speedModAmount1.get(), static_cast<int>(sampleRate * env.release.get()), isUnipolar1);
    releaseSamples = calcModRange(modValue2, env.speedModAmount1.get(), releaseSamples, isUnipolar2);

    // get growth/shrink rate from knobs
    float attackGrowthRate = env.attackShape.get();
    float decayShrinkRate = env.decayShape.get();
    float releaseShrinkRate = env.releaseShape.get();

    // release phase sets envCoeff from valueAtRelease to 0.0f
    float envCoeff;
    if (releaseCounter > -1)
    {
        if (releaseShrinkRate < 1.0f)
        {
            releaseShrinkRate = 1 / releaseShrinkRate;
            envCoeff = valueAtRelease * (1 - interpolateLog(releaseCounter, releaseSamples, releaseShrinkRate, true));
        }
        else
        {
            envCoeff = valueAtRelease * interpolateLog(releaseCounter, releaseSamples, releaseShrinkRate, false);
        }
        releaseCounter++;
    }
    else
    {
        // attack phase sets envCoeff from 0.0f to 1.0f
        if (attackDecayCounter <= attackSamples)
        {
            if (attackGrowthRate < 1.0f)
            {
                attackGrowthRate = 1 / attackGrowthRate;
                envCoeff = interpolateLog(attackDecayCounter, attackSamples, attackGrowthRate, true);
            }
            else
            {
                envCoeff = 1.0f - interpolateLog(attackDecayCounter, attackSamples, attackGrowthRate, false);
            }
            valueAtRelease = envCoeff;
            attackDecayCounter++;
        }
        else{
            
            float sustainLevel = sustain.get();
            // decay phase sets envCoeff from 1.0f to sustain level
            if (attackDecayCounter <= attackSamples + decaySamples){
                if (decayShrinkRate < 1.0f)
                {
                    decayShrinkRate = 1 / decayShrinkRate;
                    envCoeff = 1 - interpolateLog(attackDecayCounter - attackSamples, decaySamples, decayShrinkRate, true) * (1.0f - sustainLevel);
                }
                else
                {
                    envCoeff = interpolateLog(attackDecayCounter - attackSamples, decaySamples, decayShrinkRate, false) * (1.0f - sustainLevel) + sustainLevel;
                }
                valueAtRelease = envCoeff;
                attackDecayCounter++;
            }
            else // if attack and decay phase is over then sustain level
            {
                envCoeff = sustainLevel;
                valueAtRelease = envCoeff;
            }
        }
    }
    return envCoeff;
}

/**
* interpolate logarithmically from 1.0 to 0.0f in t samples
@param c counter of the specific phase
@param t number of samples after which the specific phase should be over
@param k coeff of growth/shrink, k=1 for linear
@param slow how fast is phase applied at the start
*/
float Envelope::interpolateLog(int c, int t, float k, bool slow)
{
    if (slow)
    {
        return std::exp(std::log(static_cast<float>(c) / static_cast<float>(t)) * k);
    }
    else
    {
        return std::exp(std::log(1.0f - static_cast<float>(c) / static_cast<float>(t)) * k);
    }
}