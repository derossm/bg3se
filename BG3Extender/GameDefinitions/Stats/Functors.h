#pragma once

#include <GameDefinitions/Base/Base.h>
#include <GameDefinitions/Stats/Common.h>
#include <GameDefinitions/Interrupt.h>
#include <GameDefinitions/Hit.h>
#include <GameDefinitions/Misc.h>
#include <Lua/LuaHelpers.h>

BEGIN_NS(stats)

struct Functor
{
	struct FunctorVMT
	{
		void* Destroy;
		void* ParseParams;
		void* Clone;
	};

	struct RollCondition
	{
		RollType Type;
		int32_t ConditionId{ -1 };
	};

	FunctorVMT* VMT{ nullptr };
	FixedString UniqueName;
	uint64_t Unknown1{ 0 };
	uint64_t Unknown2{ 0 };
	Array<RollCondition> RollConditions;
	int32_t StatsConditionsId{ -1 };
	PropertyContext PropertyContext{ 0 };
	uint32_t field_40{ 0 };
	ObserverType ObserverType{ ObserverType::None };
	FunctorId TypeId{ FunctorId::CustomDescription };
	FunctorFlags Flags{ 0 };

	template <class T>
	std::optional<T*> Cast()
	{
		if (TypeId == T::FunctorType) {
			return static_cast<T*>(this);
		} else {
			return {};
		}
	}
};

struct BaseFunctorExecParams
{
	FunctorExecParamsType ParamsTypeId{ 0 };
	PropertyContext PropertyContext{ 0 };
	int32_t StoryActionId{ 0 };
	ActionOriginator Originator;
	resource::GuidResourceBankBase* ClassResources{ nullptr };
	EntityHandle HistoryEntity;
	EntityHandle StatusSource;
	MultiHashMap<EntityHandle, int32_t> EntityToThothContextIndex;
	int field_98;
	bool field_9C;
};

struct FunctorExecParamsType1 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	ecs::EntityRef CasterProxy;
	ecs::EntityRef Target;
	ecs::EntityRef TargetProxy;
	glm::vec3 Position;
	bool IsFromItem;
	SpellIdWithPrototype SpellId;
	Hit Hit;
	DamageSums DamageSums;
	uint64_t field_2F8;
	uint64_t field_300;
	float SomeRadius;
	HitWith HitWith;
	uint32_t field_310;
	uint32_t field_314;
	FixedString field_26C;
	uint8_t field_31C;
};

struct FunctorExecParamsType2 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	glm::vec3 Position;
	float ExplodeRadius;
	bool IsFromItem;
	SpellIdWithPrototype SpellId;
	Hit Hit;
	DamageSums DamageSums;
	float SomeRadius;
};

struct FunctorExecParamsType3 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	ecs::EntityRef Target;
	ecs::EntityRef field_C0;
	glm::vec3 Position;
	float Distance;
};

struct FunctorExecParamsType4 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	ecs::EntityRef field_B0;
	glm::vec3 Position;
	SpellIdWithPrototype SpellId;
	Hit Hit;
	DamageSums DamageSums;
	uint64_t field_2D8;
	uint64_t field_2E0;
	uint32_t field_2E8;
	FixedString field_2EC;
	uint8_t field_2F0;
};

struct FunctorExecParamsType5 : public BaseFunctorExecParams
{
	ecs::EntityRef Owner_M;
	ecs::EntityRef Target;
	ecs::EntityRef Caster;
	ecs::EntityRef field_D0;
	ecs::EntityRef field_E0;
	glm::vec3 Position;
	bool IsFromItem;
	SpellIdWithPrototype SpellId;
	Hit Hit;
	DamageSums DamageSums;
};

struct FunctorExecParamsType6 : public BaseFunctorExecParams
{
	ecs::EntityRef Target;
	ecs::EntityRef TargetProxy;
	ecs::EntityRef Caster;
	ecs::EntityRef field_D0;
	ecs::EntityRef field_E0;
	glm::vec3 Position;
	bool IsFromItem;
	SpellIdWithPrototype SpellId;
	Hit Hit;
	DamageSums DamageSums;
};

struct FunctorExecParamsType7 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	ecs::EntityRef Target;
	bool UseCasterStats;
};

struct FunctorExecParamsType8 : public BaseFunctorExecParams
{
	ecs::EntityRef Caster;
	ecs::EntityRef Target;
};

struct FunctorExecParamsType9 : public BaseFunctorExecParams
{
	bool OnlyAllowRollAdjustments;
	ecs::EntityRef Source;
	ecs::EntityRef SourceProxy;
	ecs::EntityRef Target;
	ecs::EntityRef TargetProxy;
	ecs::EntityRef Observer;
	ecs::EntityRef ObserverProxy;
	std::optional<interrupt::ResolveData> ResolveData;
	interrupt::InterruptVariant2 Interrupt;
	Hit Hit;
	DamageSums DamageSums;
	Array<DamagePair> DamageList;
	interrupt::ExecuteResult ExecuteInterruptResult;
};



struct Functors : public Noncopyable<Functors>
{
	using ExecuteType1Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType1 * params);
	using ExecuteType2Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType2 * params);
	using ExecuteType3Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType3 * params);
	using ExecuteType4Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType4 * params);
	using ExecuteType5Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType5 * params);
	using ExecuteType6Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType6 * params);
	using ExecuteType7Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType7 * params);
	using ExecuteType8Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType8 * params);
	using ExecuteType9Proc = void (HitResult* hit, Functors* self, FunctorExecParamsType9 * params);

	struct BaseVMT
	{
		void (*Destroy)(Functors*);
		void (*ClearNextIndex)(Functors*);
		void (*Clear)(Functors*);
		void (*AddOrUpdate)(Functors*, Functor*);
		Functor* (*GetByIndex)(Functors*, int64_t);
		Functor* (*GetByName)(Functors*, FixedString const&);
		void (*Unknown_30)(Functors*);
		int64_t (*GetSize)(Functors*);
		Functor* (*GetByIndex2)(Functors*, int64_t);
		Functor* (*GetByIndex3)(Functors*, int64_t);
		void (*UpdateNameMap)(Functors*);
	};

	/*virtual ~Functors() = 0;
	virtual void ClearNextIndex() = 0;
	virtual void Clear() = 0;
	virtual void AddOrUpdate(Functor*) = 0;
	virtual Functor* GetByIndex(int64_t) = 0;
	virtual Functor* GetByName(FixedString const&) = 0;
	virtual void Unknown_30() = 0;
	virtual int64_t GetSize() = 0;
	virtual Functor* GetByIndex2(int64_t) = 0;
	virtual Functor* GetByIndex3(int64_t) = 0;
	virtual void UpdateNameMap() = 0;*/

	BaseVMT* VMT{ nullptr };
	Array<Functor*> Functors;
	MultiHashMap<FixedString, Functor*> FunctorsByName;
	int NextFunctorIndex{ 0 };
	int Unknown{ 0 };
	FixedString UniqueName;
};


// FIXME - use custom implementation of base VMT or use base VMT?
struct StatsFunctorSetImpl : public Functors
{
	/*~StatsFunctorSetImpl() override;
	void ClearNextIndex() override;
	void Clear() override;
	void AddOrUpdate(Functor*) override;
	Functor* GetByIndex(int64_t) override;
	Functor* GetByName(FixedString const&) override;
	void Unknown_30() override;
	int64_t GetSize() override;
	Functor* GetByIndex2(int64_t) override;
	Functor* GetByIndex3(int64_t) override;
	void UpdateNameMap() override;*/
};


struct CustomDescriptionFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CustomDescription;

	FixedString Description; // Arg0
};

struct ResurrectFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Resurrect;

	float Probability{ 1.0f }; // Arg0
	float HealthPercentage{ 1.0f }; // Arg1
	TargetTypeFlags Flags{ 0 };
};

struct SabotageFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Sabotage;

	int Amount{ 1 };
};

struct SummonFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Summon;

	FixedString MovingObject; // Arg1
	FixedString Arg2;
	float SpawnLifetime{ 6.0f };
	uint8_t LifetimeType{ 0 };
	MultiHashSet<FixedString> StatusesToApply;
	FixedString Arg4;
	bool Arg3;
	bool Arg9;
};

struct ForceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Force;

	FixedString Distance; // Arg0
	ForceFunctorOrigin Origin{ ForceFunctorOrigin::OriginToEntity }; // Arg1
	ForceFunctorAggression Aggression{ ForceFunctorAggression::Aggressive }; // Arg2
	StatsExpressionParamEx* Unkn{ nullptr };
	bool Arg3{ false };
	bool Arg4{ false };
};

struct DouseFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Douse;

	float field_20{ -1.0f };
	float field_24{ 1.0f };
};

struct SwapPlacesFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SwapPlaces;

	FixedString Animation; // Arg0
	bool Arg1{ false };
	bool Arg2{ false };
};

struct EqualizeFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Equalize;

	StatusHealType HealType{ StatusHealType::None }; // Arg0
};

struct PickupFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Pickup;

	FixedString Arg0;
};

struct CreateSurfaceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CreateSurface;

	FixedString SurfaceType; // Arg2 - SurfaceType or DamageType
	float Radius{ -1.0f }; // Arg0
	float Arg4{ 0.0f };
	float Duration{ -1.0f }; //Arg1
	bool IsControlledByConcentration{ true }; // Arg3
	bool Arg5{ false };
};

struct CreateConeSurfaceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CreateConeSurface;

	FixedString Arg2;
	float Arg0{ -1.0f };
	float Arg1{ -1.0f };
	bool Arg3{ false };
	bool field_55{ false };
};

struct RemoveStatusFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RemoveStatus;

	FixedString StatusId; // Arg0
};

struct ExecuteWeaponFunctorsFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::ExecuteWeaponFunctors;

	ExecuteWeaponFunctorsType WeaponType;
};

struct TeleportSourceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::TeleportSource;

	bool Arg1{ false };
	bool Arg2{ false };
};

struct SetStatusDurationFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetStatusDuration;

	FixedString StatusId; // Arg0
	float Duration{ 6.0f }; // Arg1
	bool SetIfLonger{ false }; // Arg2
};

struct UseAttackFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::UseAttack;

	bool IgnoreChecks{ false }; // Arg0
};

struct BreakConcentrationFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::BreakConcentration;

};

struct RestoreResourceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RestoreResource;

	Guid ActionResourceUUID; // Arg0
	int Hex{ 0 }; // Arg2
	int field_34{ 0 };
	StatsExpressionParam* LuaAmount{ nullptr }; // Arg1
	double Amount{ 0.0 }; // Arg1
	bool IsPercentage{ false }; // Arg1
};

struct SpawnFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Spawn;

	FixedString TemplateId; // Arg0
	FixedString Arg1;
	MultiHashSet<FixedString> StatusesToApply;
	bool Arg6{ false };
};

struct StabilizeFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Stabilize;

};

struct UnlockFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Unlock;

};

struct ResetCombatTurnFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::ResetCombatTurn;

};

struct RemoveAuraByChildStatusFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RemoveAuraByChildStatus;

	FixedString StatusId; // Arg0
};

struct ApplyStatusFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::ApplyStatus;

	FixedString StatusId; // Arg0
	FixedString StringParam; // Arg3
	STDString StatsConditions; // Arg6
	int StatsConditionsId{ -1 }; // Arg6
	int Param5{ -1 }; // Arg5
	int Param6{ -1 }; // ???
	bool Param8{ false };
	void* Arg2_DurationLuaExpression{ nullptr }; // Arg2
	bool HasParam6{ false };
};

using StatsSystem_ThrowDamageEventProc = void (void* statsSystem, void* temp5, Hit* hit, DamageSums* damageAmounts, bool a5, bool a6);

struct DealDamageFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::DealDamage;

	using ApplyDamageProc = HitResult * (HitResult* result, DealDamageFunctor* functor, ecs::EntityRef* casterHandle,
		ecs::EntityRef* targetHandle, glm::vec3* position, bool isFromItem, SpellIdWithPrototype* spellId, 
		int storyActionId, ActionOriginator* originator, resource::GuidResourceBankBase* classResourceMgr, 
		Hit* hit, DamageSums* damageSums, EntityHandle* sourceHandle2, HitWith hitWith, int conditionRollIndex,
		bool entityDamagedEventParam, __int64 a17, SpellId* spellId2);

	DamageType DamageType{ DamageType::None }; // Arg1
	DealDamageWeaponType WeaponType{ DealDamageWeaponType::None }; // Arg0
	DealDamageWeaponDamageType WeaponDamageType{ DealDamageWeaponDamageType::None }; // Arg1
	StatsExpressionParam* Damage{ nullptr };
	bool Nonlethal{ false }; // Arg3
	bool Magical{ false }; // Arg2
	int32_t field_34{ 0 }; // Arg5
};

struct UseActionResourceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::UseActionResource;

	Guid ActionResourceUUID; // Arg0
	double Amount{ 0.0 }; // Arg1
	int ResourceIndex{ 0 }; // Arg2
	bool IsPercentage{ false }; // Arg1
	bool Arg3{ false }; // Arg3
};

struct CreateExplosionFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CreateExplosion;

	FixedString SpellId; // Arg0
};

struct SurfaceChangeFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SurfaceChange;

	float Chance{ 1.0f };
	float field_24{ 0.0f };
	float field_28{ 0.0f };	
	float field_2C{ -1.0f };
	SurfaceChange SurfaceChange{ SurfaceChange::None };
};

struct ApplyEquipmentStatusFunctor : public ApplyStatusFunctor
{
	static constexpr auto FunctorType = FunctorId::ApplyEquipmentStatus;

	StatsItemSlot EquipmentSlot{ StatsItemSlot::MainHand };
};

struct RegainHitPointsFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RegainHitPoints;

	StatsExpressionParam* HitPoints{ nullptr };
	TargetTypeFlags Flags{ 0 };
};

struct UseSpellFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::UseSpell;

	FixedString SpellId; // Arg0
	bool IgnoreHasSpell{ false }; // Arg1
	bool IgnoreChecks{ false }; // Arg2
	bool Arg3{ false };
	Guid Arg4;
};

struct ExtenderFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Extender;
};

struct SummonInInventoryFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SummonInInventory;

	FixedString Arg1;
	FixedString Arg8;
	float Duration{ 0.0f }; // Arg2
	uint8_t DurationType{ 0 };
	MultiHashSet<FixedString> AdditionalArgs;
	FixedString Arg9;
	float Arg3{ 0.0f };
	bool Arg4{ false };
	bool Arg5{ false };
	bool Arg6{ false };
	bool Arg7{ false };
};

struct SpawnInInventoryFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SpawnInInventory;

	FixedString Arg1;
	FixedString Arg6;
	float Arg2{ 0.0f };
	bool Arg3{ false };
	bool Arg4{ false };
	bool Arg5{ false };
	MultiHashSet<FixedString> AdditionalArgs;
};

struct RemoveUniqueStatusFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RemoveUniqueStatus;

	FixedString StatusId; // Arg0
};

struct DisarmWeaponFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::DisarmWeapon;
};

struct SwitchDeathTypeFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SwitchDeathType;

	DeathType DeathType;
};

struct TriggerRandomCastFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::TriggerRandomCast;

	bool Arg1;
	float Arg2;
	Array<FixedString> RandomCastOutcomes;
};

struct GainTemporaryHitPointsFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::GainTemporaryHitPoints;

	StatsExpressionParam* HitPointsExpression;
};

struct FireProjectileFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::FireProjectile;

	FixedString Arg1;
};

struct ShortRestFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::ShortRest;
};

struct CreateZoneFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CreateZone;

	ZoneShape Shape; // Arg1
	FixedString Arg4;
	float Arg2;
	float Duration; // Arg3
	bool Arg5;
};

struct DoTeleportFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::DoTeleport;

	float Arg1;
};

struct RegainTemporaryHitPointsFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RegainTemporaryHitPoints;

	StatsExpressionParam* HitPoints;
};

struct RemoveStatusByLevelFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::RemoveStatusByLevel;

	struct NameOrStatusGroup
	{
		~NameOrStatusGroup()
		{
			if (!IsStatusGroup) {
				String.~FixedString();
			}
		}

		union {
			FixedString String;
			StatusGroup SG;
		};
		bool IsStatusGroup;
	};

	NameOrStatusGroup SG; // Arg1
	bool HasArg1;
	bool Arg2;
	AbilityId Ability; // Arg3
	bool HasAbility;
};

struct SurfaceClearLayerFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SurfaceClearLayer;

	MultiHashSet<SurfaceLayer8> Layers;
};

struct UnsummonFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Unsummon;
};

struct CreateWallFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CreateWall;
};

struct CounterspellFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Counterspell;
};

struct AdjustRollFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::AdjustRoll;

	StatsExpressionParam* Expression; // Arg1
	RollAdjustmentType Type; // Arg2
	DamageType DamageType; // Arg2
};

struct SpawnExtraProjectilesFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SpawnExtraProjectiles;

	FixedString Arg1;
};

struct KillFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Kill;
};

struct TutorialEventFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::TutorialEvent;

	Guid Event;
};

struct DropFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::Drop;

	FixedString Arg1;
};

struct ResetCooldownsFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::ResetCooldowns;

	SpellCooldownType CooldownType;
};

struct SetRollFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetRoll;

	int Arg1;
	RollAdjustmentType Type; // Arg2
	DamageType DamageType; // Arg2
};

struct SetDamageResistanceFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetDamageResistance;

	DamageType DamageType; // Arg1
};

struct SetRerollFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetReroll;

	uint8_t Arg1;
	bool Arg2;
};

struct SetAdvantageFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetAdvantage;
};

struct SetDisadvantageFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::SetDisadvantage;
};

struct MaximizeRollFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::MaximizeRoll;

	DamageType DamageType;
};

struct CameraWaitFunctor : public Functor
{
	static constexpr auto FunctorType = FunctorId::CameraWait;

	float Arg1;
};


END_NS()

BEGIN_NS(lua)

LUA_POLYMORPHIC(bg3se::stats::BaseFunctorExecParams)

END_NS()
