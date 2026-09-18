
#include "AuraAbilityTypes.h"


bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 1. 首先调用父类的序列化，确保基础字段（Instigator, HitResult 等）正常同步
	// 注意：不要在这里单独去搞一个新的 RepBits 去 SerializeBits！
	// 我们直接调用父类，父类内部会处理它自己的 7 位 RepBits 序列化
	
	Super::NetSerialize(Ar, Map, bOutSuccess);

	uint32 LocalBits = 0;
	if (Ar.IsSaving())
	{
		// 如果有自定义数据，点亮对应的二进制位开关
		if (bIsCriticalHit) { LocalBits |= 1 << 0; }
		if (bIsBlockedHit)  { LocalBits |= 1 << 1; }
	}

	// 把开关掩码序列化发出去（假设占用 2 位）
	Ar.SerializeBits(&LocalBits, 2);

	if (Ar.IsLoading())
	{
		// 客户端接收时，根据开关位恢复数据
		bIsCriticalHit = (LocalBits & (1 << 0)) != 0;
		bIsBlockedHit  = (LocalBits & (1 << 1)) != 0;
	}

	bOutSuccess = true;
	return true;
}
