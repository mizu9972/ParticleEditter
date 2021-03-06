#include "psvscommon.fx"
#include "Quo.fx"
#include "ParticleCommon.fx"

#define THREAD_NUM 256
#define PARTICLE_NUM_PER_THREAD 64 //CParticleと同じ定数を宣言

[numthreads(THREAD_NUM, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    for (uint i = 0; i < PARTICLE_NUM_PER_THREAD; i += 1)
    {
        const uint Id = PARTICLE_NUM_PER_THREAD * DTid.x + i;
        if (g_OutState[Id].isInitialized != 1 || (g_OutState[Id].isAlive == false && g_InState[0].isLooping == true))
        {
            ParticleInit(Id);
        }
        else
        {
            ParticleUpdate(Id);

        }
    }
}