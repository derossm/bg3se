#pragma once

BEGIN_NS(lua)

class SetProxyImplBase
{
public:
	SetProxyImplBase();
	virtual ~SetProxyImplBase();
	void Register();
	int GetRegistryIndex() const;
	virtual TypeInformation const& GetContainerType() const = 0;
	virtual TypeInformation const& GetElementType() const = 0;
	virtual bool GetElementAt(lua_State* L, CppObjectMetadata& self, unsigned int arrayIndex) = 0;
	virtual bool HasElement(lua_State* L, CppObjectMetadata& self, int luaIndex) = 0;
	virtual bool AddElement(lua_State* L, CppObjectMetadata& self, int luaIndex) = 0;
	virtual bool RemoveElement(lua_State* L, CppObjectMetadata& self, int luaIndex) = 0;
	virtual int Next(lua_State* L, CppObjectMetadata& self, int index) = 0;
	virtual unsigned Length(CppObjectMetadata& self) = 0;
	virtual bool Unserialize(lua_State* L, CppObjectMetadata& self, int index) = 0;
	virtual void Serialize(lua_State* L, CppObjectMetadata& self) = 0;

private:
	int registryIndex_{ -1 };
};

	
template <class T>
class MultiHashSetProxyImpl : public SetProxyImplBase
{
public:
	static_assert(!std::is_pointer_v<T>, "MultiHashSetProxyImpl template parameter should not be a pointer type!");

	using ElementType = T;
	using ContainerType = MultiHashSet<T>;

	~MultiHashSetProxyImpl() override
	{}

	TypeInformation const& GetContainerType() const override
	{
		return GetTypeInfo<ContainerType>();
	}

	TypeInformation const& GetElementType() const override
	{
		return GetTypeInfo<T>();
	}

	bool GetElementAt(lua_State* L, CppObjectMetadata& self, unsigned int arrayIndex) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		if (arrayIndex > 0 && arrayIndex <= obj->size()) {
			push(L, &obj->Keys[arrayIndex - 1], self.Lifetime);
			return true;
		} else {
			return false;
		}
	}

	bool HasElement(lua_State* L, CppObjectMetadata& self, int luaIndex) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		auto element = get<T>(L, luaIndex);
		return obj->FindIndex(element) != -1;
	}

	bool AddElement(lua_State* L, CppObjectMetadata& self, int luaIndex) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		auto element = get<T>(L, luaIndex);
		obj->Add(element);
		return true;
	}

	bool RemoveElement(lua_State* L, CppObjectMetadata& self, int luaIndex) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		auto element = get<T>(L, luaIndex);
		return obj->remove(element);
	}

	unsigned Length(CppObjectMetadata& self) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		return obj->Keys.Size();
	}

	int Next(lua_State* L, CppObjectMetadata& self, int key) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		if (key >= -1 && key < (int)obj->Keys.Size() - 1) {
			push(L, ++key);
			push(L, &obj->Keys[key], self.Lifetime);
			return 2;
		} else {
			return 0;
		}
	}

	bool Unserialize(lua_State* L, CppObjectMetadata& self, int index) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		if constexpr (std::is_default_constructible_v<T>) {
			lua::Unserialize(L, index, obj);
			return true;
		} else {
			return false;
		}
	}

	void Serialize(lua_State* L, CppObjectMetadata& self) override
	{
		auto obj = reinterpret_cast<ContainerType*>(self.Ptr);
		lua::Serialize(L, obj);
	}
};


class SetProxyMetatable : public LightCppObjectMetatable<SetProxyMetatable>, public Indexable, public NewIndexable,
	public Lengthable, public Iterable, public Stringifiable, public EqualityComparable
{
public:
	static constexpr MetatableTag MetaTag = MetatableTag::SetProxy;

	template <class TImpl>
	static SetProxyImplBase* GetImplementation()
	{
		static SetProxyImplBase* impl = new TImpl();
		return impl;
	}

	inline static void MakeImpl(lua_State* L, void* object, LifetimeHandle const& lifetime, SetProxyImplBase* impl)
	{
		lua_push_cppobject(L, MetatableTag::SetProxy, impl->GetRegistryIndex(), object, lifetime);
	}

	template <class T>
	inline static void Make(lua_State* L, MultiHashSet<T>* object, LifetimeHandle const& lifetime)
	{
		MakeImpl(L, object, lifetime, GetImplementation<MultiHashSetProxyImpl<T>>());
	}

	template <class T>
	inline static T::ContainerType* Get(lua_State* L, int index)
	{
		auto ptr = GetRaw(L, index, GetImplementation<T>()->GetRegistryIndex());
		return reinterpret_cast<T::ContainerType*>(ptr);
	}

	inline static SetProxyImplBase* GetImpl(CppObjectMetadata const& meta)
	{
		assert(meta.MetatableTag == MetatableTag::SetProxy);
		return GetImpl(meta.PropertyMapTag);
	}

	static int Index(lua_State* L, CppObjectMetadata& self);
	static int NewIndex(lua_State* L, CppObjectMetadata& self);
	static int Length(lua_State* L, CppObjectMetadata& self);
	static int Next(lua_State* L, CppObjectMetadata& self);
	static int ToString(lua_State* L, CppObjectMetadata& self);
	static bool IsEqual(lua_State* L, CppObjectMetadata& self, CppObjectMetadata& other);
	static char const* GetTypeName(lua_State* L, CppObjectMetadata& self);

private:
	static void* GetRaw(lua_State* L, int index, int propertyMapIndex);
	static SetProxyImplBase* GetImpl(int propertyMapIndex);
};

END_NS()

