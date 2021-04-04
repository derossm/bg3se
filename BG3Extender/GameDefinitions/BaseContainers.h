#pragma once

#include <cstdint>

#include <GameDefinitions/BaseUtilities.h>
#include <GameDefinitions/BaseMemory.h>

namespace bg3se
{
	unsigned int GetNearestLowerPrime(unsigned int num);
	unsigned int GetNearestMultiHashMapPrime(unsigned int num);

	template <class T>
	struct ContiguousIterator
	{
		T* Ptr;

		ContiguousIterator(T* p) : Ptr(p) {}

		ContiguousIterator operator ++ ()
		{
			ContiguousIterator<T> it(Ptr);
			Ptr++;
			return it;
		}

		ContiguousIterator& operator ++ (int)
		{
			Ptr++;
			return *this;
		}

		bool operator == (ContiguousIterator const& it)
		{
			return it.Ptr == Ptr;
		}

		bool operator != (ContiguousIterator const& it)
		{
			return it.Ptr != Ptr;
		}

		T& operator * ()
		{
			return *Ptr;
		}

		T* operator -> ()
		{
			return Ptr;
		}
	};


	template <class T>
	struct ContiguousConstIterator
	{
		T const* Ptr;

		ContiguousConstIterator(T const* p) : Ptr(p) {}

		ContiguousConstIterator operator ++ ()
		{
			ContiguousConstIterator<T> it(Ptr);
			Ptr++;
			return it;
		}

		ContiguousConstIterator& operator ++ (int)
		{
			Ptr++;
			return *this;
		}

		bool operator == (ContiguousConstIterator const& it)
		{
			return it.Ptr == Ptr;
		}

		bool operator != (ContiguousConstIterator const& it)
		{
			return it.Ptr != Ptr;
		}

		T const& operator * ()
		{
			return *Ptr;
		}

		T const* operator -> ()
		{
			return Ptr;
		}
	};


	template <class TKey, class TValue>
	class Map : public Noncopyable<Map<TKey, TValue>>
	{
	public:
		struct Node
		{
			Node* Next{ nullptr };
			TKey Key;
			TValue Value;
		};

		class Iterator
		{
		public:
			Iterator(Map& map) 
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}
			
			Iterator(Map& map, Node** node, Node* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			Iterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			Iterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey & Key () const
			{
				return Element->Key;
			}

			TKey & Value () const
			{
				return Element->Value;
			}

			Node& operator * () const
			{
				return *Element;
			}

			Node& operator -> () const
			{
				return *Element;
			}

		private:
			Node** Node, ** NodeListEnd;
			Map<TKey, TValue>::Node* Element;
		};

		class ConstIterator
		{
		public:
			ConstIterator(Map const& map)
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}

			ConstIterator(Map const& map, Node* const* node, Node const* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			ConstIterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			ConstIterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey const& Key() const
			{
				return Element->Key;
			}

			TKey const& Value() const
			{
				return Element->Value;
			}

			Node const& operator * () const
			{
				return *Element;
			}

			Node const& operator -> () const
			{
				return *Element;
			}

		private:
			Node* const * Node, * const * NodeListEnd;
			Map<TKey, TValue>::Node const* Element;
		};

		Map() {}

		Map(uint32_t hashSize)
		{
			Init(hashSize);
		}

		~Map()
		{
			Clear();
		}

		void Init(uint32_t hashSize)
		{
			HashSize = hashSize;
			HashTable = GameAllocArray<Node*>(hashSize);
			ItemCount = 0;
			memset(HashTable, 0, sizeof(Node*) * hashSize);
		}

		void Clear()
		{
			ItemCount = 0;
			for (uint32_t i = 0; i < HashSize; i++) {
				auto item = HashTable[i];
				if (item != nullptr) {
					FreeHashChain(item);
					HashTable[i] = nullptr;
				}
			}
		}

		void FreeHashChain(Node* node)
		{
			do {
				auto next = node->Next;
				GameDelete(node);
				node = next;
			} while (node != nullptr);
		}

		TValue* Insert(TKey const& key, TValue const& value)
		{
			auto nodeValue = Insert(key);
			*nodeValue = value;
			return nodeValue;
		}

		TValue* Insert(TKey const& key)
		{
			auto item = HashTable[Hash(key) % HashSize];
			auto last = item;
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				last = item;
				item = item->Next;
			}

			auto node = GameAlloc<Node>();
			node->Next = nullptr;
			node->Key = key;

			if (last == nullptr) {
				HashTable[Hash(key) % HashSize] = node;
			}
			else {
				last->Next = node;
			}

			ItemCount++;
			return &node->Value;
		}

		TValue* Find(TKey const& key) const
		{
			auto item = HashTable[Hash(key) % HashSize];
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				item = item->Next;
			}

			return nullptr;
		}

		TKey* FindByValue(TValue const& value) const
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					if (value == item->Value) {
						return &item->Key;
					}

					item = item->Next;
				}
			}

			return nullptr;
		}

		template <class Visitor>
		void Iterate(Visitor visitor)
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

		template <class Visitor>
		void Iterate(Visitor visitor) const
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

		Iterator begin()
		{
			return Iterator(*this);
		}

		Iterator end()
		{
			return Iterator(*this, HashTable + HashSize, nullptr);
		}

		Iterator begin() const
		{
			return ConstIterator(*this);
		}

		Iterator end() const
		{
			return ConstIterator(*this, HashTable + HashSize, nullptr);
		}

		inline uint32_t Count() const
		{
			return ItemCount;
		}

	private:
		uint32_t HashSize{ 0 };
		Node** HashTable{ nullptr };
		uint32_t ItemCount{ 0 };
	};

	template <class TKey, class TValue>
	class RefMap : public Noncopyable<RefMap<TKey, TValue>>
	{
	public:
		struct Node
		{
			Node* Next{ nullptr };
			TKey Key;
			TValue Value;
		};

		class Iterator
		{
		public:
			Iterator(RefMap& map) 
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}
			
			Iterator(RefMap& map, Node** node, Node* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			Iterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			Iterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey & Key () const
			{
				return Element->Key;
			}

			TKey & Value () const
			{
				return Element->Value;
			}

			Node& operator * () const
			{
				return *Element;
			}

			Node& operator -> () const
			{
				return *Element;
			}

		private:
			Node** Node, ** NodeListEnd;
			RefMap<TKey, TValue>::Node* Element;
		};

		class ConstIterator
		{
		public:
			ConstIterator(RefMap const& map)
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}

			ConstIterator(RefMap const& map, Node* const* node, Node const* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			ConstIterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			ConstIterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey const& Key() const
			{
				return Element->Key;
			}

			TKey const& Value() const
			{
				return Element->Value;
			}

			Node const& operator * () const
			{
				return *Element;
			}

			Node const& operator -> () const
			{
				return *Element;
			}

		private:
			Node* const * Node, * const * NodeListEnd;
			RefMap<TKey, TValue>::Node const* Element;
		};

		RefMap(uint32_t hashSize = 31)
			: ItemCount(0), HashSize(hashSize)
		{
			HashTable = GameAllocArray<Node*>(hashSize);
			memset(HashTable, 0, sizeof(Node*) * hashSize);
		}

		~RefMap()
		{
			if (HashTable != nullptr) {
				GameFree(HashTable);
			}
		}

		Iterator begin()
		{
			return Iterator(*this);
		}

		Iterator end()
		{
			return Iterator(*this, HashTable + HashSize, nullptr);
		}

		Iterator begin() const
		{
			return ConstIterator(*this);
		}

		Iterator end() const
		{
			return ConstIterator(*this, HashTable + HashSize, nullptr);
		}

		inline uint32_t Count() const
		{
			return ItemCount;
		}

		void Clear()
		{
			ItemCount = 0;
			for (uint32_t i = 0; i < HashSize; i++) {
				auto item = HashTable[i];
				if (item != nullptr) {
					FreeHashChain(item);
					HashTable[i] = nullptr;
				}
			}
		}

		void FreeHashChain(Node* node)
		{
			do {
				auto next = node->Next;
				GameDelete(node);
				node = next;
			} while (node != nullptr);
		}

		TValue* Find(TKey const& key) const
		{
			auto item = HashTable[Hash(key) % HashSize];
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				item = item->Next;
			}

			return nullptr;
		}

		TValue* Insert(TKey const& key, TValue const& value)
		{
			auto nodeValue = Insert(key);
			*nodeValue = value;
			return nodeValue;
		}

		TValue* Insert(TKey const& key)
		{
			auto item = HashTable[Hash(key) % HashSize];
			auto last = item;
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				last = item;
				item = item->Next;
			}

			auto node = GameAlloc<Node>();
			node->Next = nullptr;
			node->Key = key;

			if (last == nullptr) {
				HashTable[Hash(key) % HashSize] = node;
			}
			else {
				last->Next = node;
			}

			ItemCount++;
			return &node->Value;
		}

		template <class Visitor>
		void Iterate(Visitor visitor)
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

	private:
		uint32_t ItemCount{ 0 };
		uint32_t HashSize{ 0 };
		Node** HashTable{ nullptr };
	};


	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct CompactSet
	{
		T* Buf{ nullptr };
		uint32_t Capacity{ 0 };
		uint32_t Size{ 0 };

		inline CompactSet() {}

		CompactSet(CompactSet const& other)
		{
			Reallocate(other.Size);
			Size = other.Size;
			for (uint32_t i = 0; i < other.Size; i++) {
				new (Buf + i) T(other.Buf[i]);
			}
		}

		~CompactSet()
		{
			if (Buf) {
				Clear();
				FreeBuffer(Buf);
			}
		}

		CompactSet& operator = (CompactSet const& other)
		{
			Clear();
			Reallocate(other.Size);
			Size = other.Size;
			for (uint32_t i = 0; i < other.Size; i++) {
				new (Buf + i) T(other.Buf[i]);
			}
			return *this;
		}

		inline T const& operator [] (uint32_t index) const
		{
			return Buf[index];
		}

		inline T& operator [] (uint32_t index)
		{
			return Buf[index];
		}

		void FreeBuffer(void* buf)
		{
			if (StoreSize) {
				if (buf != nullptr) {
					Allocator::Free((void*)((std::ptrdiff_t)buf - 8));
				}
			} else {
				if (buf != nullptr) {
					Allocator::Free(buf);
				}
			}
		}

		void RawReallocate(uint32_t newCapacity)
		{
			if (newCapacity > 0) {
				if (StoreSize) {
					auto newBuf = Allocator::Alloc(newCapacity * sizeof(T) + 8);
					*(uint64_t*)newBuf = newCapacity;

					Buf = (T*)((std::ptrdiff_t)newBuf + 8);
				}
				else {
					Buf = Allocator::New<T>(newCapacity);
				}
			} else {
				Buf = nullptr;
			}

			Capacity = newCapacity;
		}

		void Reallocate(uint32_t newCapacity)
		{
			auto oldBuf = Buf;
			RawReallocate(newCapacity);

			for (uint32_t i = 0; i < std::min(Size, newCapacity); i++) {
				new (Buf + i) T(oldBuf[i]);
			}

			for (uint32_t i = 0; i < Size; i++) {
				oldBuf[i].~T();
			}

			FreeBuffer(oldBuf);
		}

		void Remove(uint32_t index)
		{
			if (index >= Size) {
				ERR("Tried to remove out-of-bounds index %d!", index);
				return;
			}

			for (auto i = index; i < Size - 1; i++) {
				Buf[i] = Buf[i + 1];
			}

			Buf[Size - 1].~T();
			Size--;
		}

		void Clear()
		{
			for (uint32_t i = 0; i < Size; i++) {
				Buf[i].~T();
			}

			Size = 0;
		}

		ContiguousIterator<T> begin()
		{
			return ContiguousIterator<T>(Buf);
		}

		ContiguousConstIterator<T> begin() const
		{
			return ContiguousConstIterator<T>(Buf);
		}

		ContiguousIterator<T> end()
		{
			return ContiguousIterator<T>(Buf + Size);
		}

		ContiguousConstIterator<T> end() const
		{
			return ContiguousConstIterator<T>(Buf + Size);
		}
	};

	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct Set : public CompactSet<T, Allocator, StoreSize>
	{
		uint64_t CapacityIncrementSize{ 0 };

		uint32_t CapacityIncrement() const
		{
			if (CapacityIncrementSize != 0) {
				return Capacity + (uint32_t)CapacityIncrementSize;
			}
			else if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			new (&Buf[Size++]) T(value);
		}

		void InsertAt(uint32_t index, T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			for (auto i = Size; i > index; i--) {
				Buf[i] = Buf[i - 1];
			}

			Buf[index] = value;
			Size++;
		}
	};

	template <class T, class Allocator = GameMemoryAllocator>
	struct PrimitiveSmallSet : public CompactSet<T, Allocator, false>
	{
		virtual ~PrimitiveSmallSet() {}

		uint32_t CapacityIncrement() const
		{
			if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			new (&Buf[Size++]) T(value);
		}
	};

	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct ObjectSet : public Set<T, Allocator, StoreSize>
	{
	};

	template <class T, class Allocator = GameMemoryAllocator>
	struct PrimitiveSet : public ObjectSet<T, Allocator, false>
	{
	};

	template <unsigned TDWords>
	struct BitArray
	{
		uint32_t Bits[TDWords];

		inline bool Set(uint32_t index)
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			Bits[(index - 1) >> 5] |= (1 << ((index - 1) & 0x1f));
			return true;
		}

		inline bool Clear(uint32_t index)
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			Bits[(index - 1) >> 5] &= ~(1 << ((index - 1) & 0x1f));
			return true;
		}

		inline bool IsSet(uint32_t index) const
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			return (Bits[(index - 1) >> 5] & (1 << ((index - 1) & 0x1f))) != 0;
		}
	};

	template <class T>
	struct Array
	{
		T* Buf{ nullptr };
		unsigned int Capacity{ 0 };
		unsigned int Unknown{ 0 };
		unsigned int Size{ 0 };
		unsigned int Unknown2{ 0 };

		inline Array() {}

		Array(Array const& a)
		{
			CopyFrom(a);
		}

		~Array()
		{
			if (Buf) {
				Clear();
				GameFree(Buf);
			}
		}

		Array& operator =(Array const& a)
		{
			CopyFrom(a);
			return *this;
		}

		void CopyFrom(Array const& a)
		{
			Unknown = a.Unknown;
			Unknown2 = a.Unknown2;
			Clear();

			if (a.Size > 0) {
				Reallocate(a.Size);
				Size = a.Size;
				for (uint32_t i = 0; i < Size; i++) {
					new (Buf + i) T(a[i]);
				}
			}
		}

		inline T const& operator [] (uint32_t index) const
		{
			return Buf[index];
		}

		inline T& operator [] (uint32_t index)
		{
			return Buf[index];
		}

		uint32_t CapacityIncrement() const
		{
			if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Clear()
		{
			for (uint32_t i = 0; i < Size; i++) {
				Buf[i].~T();
			}

			Size = 0;
		}

		void Reallocate(uint32_t newCapacity)
		{
			auto newBuf = GameMemoryAllocator::NewRaw<T>(newCapacity);
			for (uint32_t i = 0; i < std::min(Size, newCapacity); i++) {
				new (newBuf + i) T(Buf[i]);
			}

			if (Buf != nullptr) {
				for (uint32_t i = 0; i < Size; i++) {
					Buf[i].~T();
				}

				GameFree(Buf);
			}

			Buf = newBuf;
			Capacity = newCapacity;
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			new (&Buf[Size++]) T(value);
		}

		void Remove(uint32_t index)
		{
			if (index >= Size) {
				ERR("Tried to remove out-of-bounds index %d!", index);
				return;
			}

			for (auto i = index; i < Size - 1; i++) {
				Buf[i] = Buf[i + 1];
			}

			Buf[Size - 1].~T();
			Size--;
		}

		ContiguousIterator<T> begin()
		{
			return ContiguousIterator<T>(Buf);
		}

		ContiguousConstIterator<T> begin() const
		{
			return ContiguousConstIterator<T>(Buf);
		}

		ContiguousIterator<T> end()
		{
			return ContiguousIterator<T>(Buf + Size);
		}

		ContiguousConstIterator<T> end() const
		{
			return ContiguousConstIterator<T>(Buf + Size);
		}
	};

	template <class T>
	struct VirtualArray : public Array<T>
	{
		inline virtual ~VirtualArray() {};
	};

	// Special hashing needed for FixedStrings in the new hash table

	template <class T>
	inline uint64_t MultiHashMapHash(T const& v)
	{
		return Hash(v);
	}

	template <>
	inline uint64_t MultiHashMapHash<FixedString>(FixedString const& v)
	{
		return v.GetHash();
	}

	template <class T>
	struct MultiHashSet
	{
		int32_t* HashKeys{ nullptr };
		uint32_t NumHashKeys{ 0 };
		Array<int32_t> NextIds;
		Array<T> Keys;

		MultiHashSet()
		{}

		MultiHashSet(MultiHashSet const& other)
			: HashKeys(nullptr), NumHashKeys(other.NumHashKeys), NextIds(other.NextIds), Keys(other.Keys)
		{
			if (other.HashKeys) {
				HashKeys = GameAllocArray<int>(NumHashKeys);
				std::copy(other.HashKeys, other.HashKeys + other.NumHashKeys, HashKeys);
			}
		}

		~MultiHashSet()
		{
			if (HashKeys) {
				GameFree(HashKeys);
			}
		}

		MultiHashSet& operator =(MultiHashSet const& other)
		{
			if (HashKeys) {
				GameFree(HashKeys);
				HashKeys = nullptr;
			}

			NextIds = other.NextIds;
			Keys = other.Keys;

			NumHashKeys = other.NumHashKeys;
			if (other.HashKeys) {
				HashKeys = GameAllocArray<int>(NumHashKeys);
				std::copy(other.HashKeys, other.HashKeys + other.NumHashKeys, HashKeys);
			}

			return *this;
		}

		int FindIndex(T const& key) const
		{
			if (NumHashKeys == 0) return -1;

			auto keyIndex = HashKeys[(uint32_t)MultiHashMapHash(key) % NumHashKeys];
			while (keyIndex >= 0) {
				if (Keys[keyIndex] == key) return keyIndex;
				keyIndex = NextIds[keyIndex];
			}

			return -1;
		}

		bool Contains(T const& key) const
		{
			return FindIndex(key) != -1;
		}

		void Clear()
		{
			std::fill(HashKeys, HashKeys + NumHashKeys, -1);
			NextIds.Clear();
			Keys.Clear();
		}

		int Add(T const& key)
		{
			auto index = FindIndex(key);
			if (index != -1) {
				return index;
			}

			int keyIdx = (int)Keys.Size;
			Keys.Add(key);
			NextIds.Add(-1);

			if (NumHashKeys >= Keys.Size * 2) {
				InsertToHashMap(key, keyIdx);
			} else {
				ResizeHashMap(2 * (unsigned)Keys.Size);
			}

			return keyIdx;
		}

		ContiguousIterator<T> begin()
		{
			return Keys.begin();
		}

		ContiguousConstIterator<T> begin() const
		{
			return Keys.begin();
		}

		ContiguousIterator<T> end()
		{
			return Keys.end();
		}

		ContiguousConstIterator<T> end() const
		{
			return Keys.end();
		}

	private:
		void InsertToHashMap(T const& key, int keyIdx)
		{
			auto bucket = (uint32_t)MultiHashMapHash(key) % NumHashKeys;
			auto prevKeyIdx = HashKeys[bucket];
			if (prevKeyIdx < 0) {
				prevKeyIdx = -2 - (int)bucket;
			}

			NextIds[keyIdx] = prevKeyIdx;
			HashKeys[bucket] = keyIdx;
		}

		void ResizeHashMap(unsigned int newSize)
		{
			auto numBuckets = GetNearestMultiHashMapPrime(newSize);
			if (HashKeys) {
				GameFree(HashKeys);
			}

			HashKeys = GameAllocArray<int32_t>(numBuckets, -1);
			NumHashKeys = numBuckets;
			for (unsigned k = 0; k < Keys.Size; k++) {
				InsertToHashMap(Keys[k], k);
			}
		}
	};

	template <class T>
	struct VirtualMultiHashSet : public MultiHashSet<T>
	{
		virtual inline void Dummy() {}
	};

	template <class TKey, class TValue>
	struct MultiHashMap : public MultiHashSet
	{
		TValue* Values{ nullptr };
		int32_t NumValues{ 0 };

		MultiHashMap()
		{}

		MultiHashMap(MultiHashMap const& other)
			: MultiHashSet(other)
		{
			NumValues = other.NumValues;
			if (other.Values) {
				Values = GameAllocArray<TValue>(NumValues);
				for (auto i = 0; i < NumValues < i++) {
					new (Values + i) TValue(other.Values[i]);
				}
			}
		}

		~MultiHashMap()
		{
			FreeValues();
		}

		MultiHashMap& operator =(MultiHashMap const& other)
		{
			FreeValues();

			NumValues = other.NumValues;
			if (other.Values) {
				Values = GameAllocArray<TValue>(NumValues);
				for (auto i = 0; i < NumValues < i++) {
					new (Values + i) TValue(other.Values[i]);
				}
			}

			return *this;
		}

		std::optional<TValue const*> Find(TKey const& key) const
		{
			auto index = FindIndex(key);
			if (index == -1) {
				return {};
			} else {
				return Values + index;
			}
		}

		std::optional<TValue*> Find(TKey const& key)
		{
			auto index = FindIndex(key);
			if (index == -1) {
				return {};
			} else {
				return Values + index;
			}
		}

		void Set(TKey const& key, TValue const& value)
		{
			auto index = FindIndex(key);
			if (index == -1) {
				index = Add(key);
				if (NumValues <= index) {

				}
			}

			Values[index] = value;
		}

	private:
		void ResizeValues(int32_t newSize)
		{
			auto numBuckets = GetNearestMultiHashMapPrime(newSize);
			if (HashKeys) {
				GameFree(HashKeys);
			}

			HashKeys = GameAllocArray<int32_t>(numBuckets, -1);
			NumHashKeys = numBuckets;
			for (unsigned k = 0; k < Keys.Size; k++) {
				InsertToHashMap(Keys[k], k);
			}
		}

		void FreeValues()
		{
			if (Values) {
				for (auto i = 0; i < NumValues; i++) {
					Values[i].~TValue();
				}

				GameFree(Values);
			}
		}
	};

	template <class TKey, class TValue>
	struct VirtualMultiHashMap : public MultiHashMap<TKey, TValue>
	{
		virtual inline void Dummy() {}
	};
}
