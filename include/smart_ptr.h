// smart_ptr - simple reference counted pointer.
// Copyright (c) 2013, oddman (https://github.com/oddman2017/SmartPointer/)
// ISC License - see LICENSE file for details.

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

// C++11 feature detection
#if __cplusplus >= 201103L || _MSC_VER >= 1900
	#define SMART_PTR_NULLPTR nullptr
	#define SMART_PTR_NOEXCEPT noexcept
	#define SMART_PTR_EXPLICIT_BOOL explicit operator bool
	#define SMART_PTR_SUPPORT_MOVE 1
	#include <utility>
#else
	#define SMART_PTR_NULLPTR 0
	#define SMART_PTR_NOEXCEPT throw()
	#define SMART_PTR_EXPLICIT_BOOL operator unspecified_bool_type
	#define SMART_PTR_SUPPORT_MOVE 0
#endif

namespace smart_ptr
{
	class ref_count
	{
	public:
		ref_count() : m_strong_ref_count(1), m_weak_ref_count(0)
		{
		}

		~ref_count()
		{
		}

		// increment use count
		int inc_ref()
		{
			return ++m_strong_ref_count;
		}

		// increment weak reference count
		int inc_weak_ref()
		{
			return ++m_weak_ref_count;
		}

		// decrement use count
		int dec_ref()
		{
			return --m_strong_ref_count;
		}

		// decrement weak reference count
		int dec_weak_ref()
		{
			return --m_weak_ref_count;
		}

		// return use count
		int get_ref_count() const
		{
			return m_strong_ref_count;
		}

		// return true if _Uses == 0
		bool expired() const
		{
			return (get_ref_count() == 0);
		}

		int get_weak_ref_count() const
		{
			return m_weak_ref_count;
		}

	private:
		int m_strong_ref_count;
		int m_weak_ref_count;
	};

#if defined(WIN32) || defined(_WIN32)
	template <class T>
	class _NoAddRefReleaseOnComPtr : public T
	{
	private:
		virtual unsigned long __stdcall AddRef(void) = 0;
		virtual unsigned long __stdcall Release(void) = 0;
	};
#endif // defined(WIN32) || defined(_WIN32)

	// Visible before base_ptr so shared_ptr can befriend the casts, and so the
	// friend declaration matches its later definition on VC8/VS2005.
	template <class T, typename mem_mgr> class shared_ptr;
	template <class T, typename mem_mgr> class weak_ptr;

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> static_pointer_cast(const shared_ptr<Q, mem_mgr>& sp);
	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> dynamic_pointer_cast(const shared_ptr<Q, mem_mgr>& sp);
	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> const_pointer_cast(const shared_ptr<Q, mem_mgr>& sp);
	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> reinterpret_pointer_cast(const shared_ptr<Q, mem_mgr>& sp);

	// base class for shared_ptr and weak_ptr
	template <class T, bool is_strong, typename mem_mgr>
	class base_ptr
	{
	public:
		explicit base_ptr(T* p = 0) : m_counter(0), m_ptr(0)
		{
			if (p)
			{
				if (is_strong)
				{
					// allocate a new ref_count before taking ownership of p;
					// if new throws, m_ptr is still 0 and p will not be
					// double-deleted by any partially-constructed object.
					m_counter = new ref_count;
				}
				m_ptr = p;
			}
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		base_ptr(const base_ptr& rhs) : m_counter(0), m_ptr(0)
		{
			acquire(rhs);
		}
#endif

		template <class Q, bool b, typename mem_mgr2>
		base_ptr(const base_ptr<Q, b, mem_mgr2>& rhs) : m_counter(0), m_ptr(0)
		{
			acquire(rhs);
		}

#if SMART_PTR_SUPPORT_MOVE
		// Defined in base_ptr because base_ptr befriends its own instantiations,
		// granting access to rhs's protected members across types. shared_ptr and
		// weak_ptr delegate here since their own members lack that access.
		template <class Q, bool b, typename mem_mgr2>
		base_ptr(base_ptr<Q, b, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT : m_counter(0), m_ptr(0)
		{
			m_counter = rhs.m_counter;
			m_ptr = static_cast<T*>(rhs.m_ptr);
			rhs.m_counter = 0;
			rhs.m_ptr = 0;
		}
#endif

		virtual ~base_ptr()
		{
			release();
		}

		T& operator*() const SMART_PTR_NOEXCEPT { return *m_ptr; }

#if __cplusplus >= 201103L || _MSC_VER >= 1900
		// C++11: explicit bool conversion for conditional statements
		SMART_PTR_EXPLICIT_BOOL() const SMART_PTR_NOEXCEPT { return m_ptr != 0; }
#else
		// C++03: safe bool idiom to support if(sp) without allowing int x = sp;
		typedef T* (base_ptr::*unspecified_bool_type)() const;
		operator unspecified_bool_type() const SMART_PTR_NOEXCEPT
		{
			return m_ptr ? &base_ptr::get : 0;
		}
#endif

		// Negation operator: if (!sp) { ... }
		bool operator!() const SMART_PTR_NOEXCEPT
		{
			return !m_ptr;
		}

#if defined(WIN32) || defined(_WIN32)
		_NoAddRefReleaseOnComPtr<T>* operator->() const SMART_PTR_NOEXCEPT
		{
			return (_NoAddRefReleaseOnComPtr<T>*)m_ptr;
		}
#else
		T* operator->() const SMART_PTR_NOEXCEPT
		{
			return m_ptr;
		}
#endif // defined(WIN32) || defined(_WIN32)
		T* get() const SMART_PTR_NOEXCEPT
		{
			return m_ptr;
		}

		bool unique() const SMART_PTR_NOEXCEPT
		{
			return (m_counter ? (1 == m_counter->get_ref_count()) : true);
		}

		void reset(T* p = 0)
		{
			base_ptr<T, is_strong, mem_mgr> ptr(p);
			reset(ptr);
		}

		template <class Q, bool b, typename mem_mgr2>
		void reset(const base_ptr<Q, b, mem_mgr2>& rhs)
		{
			if ((void*)this != (void*)&rhs)
			{
				release();
				acquire(rhs);
			}
		}

		int use_count(void) const
		{
			int nRs = 0;
			if (m_counter)
			{
				nRs = m_counter->get_ref_count();
			}
			return nRs;
		}

		// swap pointers (same-type only: a different mem_mgr would call the
		// wrong deleter on destruction)
		void swap(base_ptr& rhs)
		{
			private_swap(m_counter, rhs.m_counter);
			private_swap(m_ptr, rhs.m_ptr);
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		base_ptr& operator=(const base_ptr& rhs)
		{
			reset(rhs);
			return *this;
		}
#endif

		template <class Q, bool b, typename mem_mgr2>
		base_ptr& operator=(const base_ptr<Q, b, mem_mgr2>& rhs)
		{
			reset(rhs);
			return *this;
		}

#if SMART_PTR_SUPPORT_MOVE
		template <class Q, bool b, typename mem_mgr2>
		base_ptr& operator=(base_ptr<Q, b, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT
		{
			if ((void*)this != (void*)&rhs)
			{
				release();
				m_counter = rhs.m_counter;
				m_ptr = static_cast<T*>(rhs.m_ptr);
				rhs.m_counter = 0;
				rhs.m_ptr = 0;
			}
			return *this;
		}
#endif

	protected:
		ref_count* m_counter;
		T* m_ptr;

		template <typename TP1, typename TP2>
		static void private_swap(TP1& obj1, TP2& obj2)
		{
			TP1 tmp = obj1;
			obj1 = static_cast<TP1>(obj2);
			obj2 = static_cast<TP2>(tmp);
		}

		template <class Q, bool b, typename mem_mgr2>
		void acquire(const base_ptr<Q, b, mem_mgr2>& rhs) SMART_PTR_NOEXCEPT
		{
			if (rhs.m_counter)
			{
				if (is_strong)
				{
					// strong: only acquire if managed object is still alive
					if (rhs.m_counter->get_ref_count())
					{
						m_counter = rhs.m_counter;
						m_counter->inc_ref();
						m_ptr = static_cast<T*>(rhs.m_ptr);
					}
				}
				else
				{
					// weak: always acquire as long as counter exists,
					// even if the managed object has been destroyed
					m_counter = rhs.m_counter;
					m_counter->inc_weak_ref();
					m_ptr = static_cast<T*>(rhs.m_ptr);
				}
			}
		}

		// decrement the count, delete if it is 0
		void release(void)
		{
			if (m_counter)
			{
				// Save counter to local var before potential delete,
				// prevents use-after-free warnings with -O2
				ref_count* counter = m_counter;
				m_counter = 0;

				if (is_strong)
				{
					if (0 == counter->dec_ref())
					{
						mem_mgr::deallocate(m_ptr);
					}
				}
				else
				{
					counter->dec_weak_ref();
				}
				if (0 == counter->get_ref_count() && 0 == counter->get_weak_ref_count())
				{
					delete counter;
				}
			}
			m_ptr = 0;
		}
#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q, bool b, typename mem_mgr2>
		friend class base_ptr;
		// owner_before compares another instantiation's m_counter, so the family needs
		// cross-instantiation access to the protected members.
		template <class Q, typename mem_mgr2>
		friend class shared_ptr;
		template <class Q, typename mem_mgr2>
		friend class weak_ptr;
#endif
	};

	template <class T, bool bx, class Q, bool by, typename mem_mgr1, typename mem_mgr2>
	bool operator<(const base_ptr<T, bx, mem_mgr1>& lhs, const base_ptr<Q, by, mem_mgr2>& rhs)
	{
		// test if left pointer < right pointer
		return lhs.get() < rhs.get();
	}

	template <class T, typename mem_mgr>
	class weak_ptr;

	template <typename T>
	class std_mem_mgr
	{
	public:
		// rebind<U>::other yields std_mem_mgr<U>. Used by the *_pointer_cast()
		// helpers so that, e.g., static_pointer_cast<Base>(shared_ptr<Derived>)
		// rebinds the deleter from std_mem_mgr<Derived> to std_mem_mgr<Base>;
		// without this, release() would call deallocate(Base*) on a deleter
		// whose parameter type is Derived*, which fails to type-check.
		template <typename U>
		struct rebind
		{
			typedef std_mem_mgr<U> other;
		};

		static void deallocate(T* p) { delete p; }
		static T* allocate(void) { return new T(); }
		template <typename A1>
		static T* allocate(A1 const& a1) { return new T(a1); }
		template <typename A1, typename A2>
		static T* allocate(A1 const& a1, A2 const& a2) { return new T(a1, a2); }
		template <typename A1, typename A2, typename A3>
		static T* allocate(A1 const& a1, A2 const& a2, A3 const& a3) { return new T(a1, a2, a3); }
		template <typename A1, typename A2, typename A3, typename A4>
		static T* allocate(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4) { return new T(a1, a2, a3, a4); }
		template <typename A1, typename A2, typename A3, typename A4, typename A5>
		static T* allocate(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5) { return new T(a1, a2, a3, a4, a5); }
		template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
		static T* allocate(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6) { return new T(a1, a2, a3, a4, a5, a6); }
	};

	template <class T, typename mem_mgr = std_mem_mgr<T> >
	class shared_ptr : public base_ptr<T, true, mem_mgr>
	{
		typedef base_ptr<T, true, mem_mgr> baseClass;

	private:
		// Pointer casts share another shared_ptr's control block directly rather than
		// through acquire(), so they need access to the protected members.
		template <class TT, class QQ, typename mm>
		friend shared_ptr<TT, typename mm::template rebind<TT>::other> static_pointer_cast(const shared_ptr<QQ, mm>& sp);
		template <class TT, class QQ, typename mm>
		friend shared_ptr<TT, typename mm::template rebind<TT>::other> dynamic_pointer_cast(const shared_ptr<QQ, mm>& sp);
		template <class TT, class QQ, typename mm>
		friend shared_ptr<TT, typename mm::template rebind<TT>::other> const_pointer_cast(const shared_ptr<QQ, mm>& sp);
		template <class TT, class QQ, typename mm>
		friend shared_ptr<TT, typename mm::template rebind<TT>::other> reinterpret_pointer_cast(const shared_ptr<QQ, mm>& sp);

	public:
		shared_ptr() : baseClass(0)
		{
		}

		template<class Q>
		explicit shared_ptr(Q* p) : baseClass(static_cast<T*>(p))
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		shared_ptr(const shared_ptr& rhs) : baseClass(rhs)
		{
		}
#endif

		template <class Q, typename mem_mgr2>
		shared_ptr(const shared_ptr<Q, mem_mgr2>& rhs) : baseClass(rhs)
		{
		}

#if SMART_PTR_SUPPORT_MOVE
		// Move constructor
		shared_ptr(shared_ptr&& rhs) SMART_PTR_NOEXCEPT : baseClass(0)
		{
			this->m_counter = rhs.m_counter;
			this->m_ptr = rhs.m_ptr;
			rhs.m_counter = 0;
			rhs.m_ptr = 0;
		}

		// Move assignment
		shared_ptr& operator=(shared_ptr&& rhs) SMART_PTR_NOEXCEPT
		{
			if (this != &rhs)
			{
				this->release();
				this->m_counter = rhs.m_counter;
				this->m_ptr = rhs.m_ptr;
				rhs.m_counter = 0;
				rhs.m_ptr = 0;
			}
			return *this;
		}

		// Delegates to base_ptr, which holds the cross-type access rights to steal
		// rhs's members.
		template <class Q, typename mem_mgr2>
		shared_ptr(shared_ptr<Q, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT
			: baseClass(static_cast<base_ptr<Q, true, mem_mgr2>&&>(rhs))
		{
		}

		template <class Q, typename mem_mgr2>
		shared_ptr& operator=(shared_ptr<Q, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT
		{
			baseClass::operator=(static_cast<base_ptr<Q, true, mem_mgr2>&&>(rhs));
			return *this;
		}
#endif

		// construct shared_ptr object that owns resource *rhs
		template <class Q, typename mem_mgr2>
		explicit shared_ptr(const weak_ptr<Q, mem_mgr2>& rhs) : baseClass(rhs)
		{
		}

		~shared_ptr()
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		shared_ptr& operator=(const shared_ptr& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}
#endif

		template <class Q, typename mem_mgr2>
		shared_ptr& operator=(const shared_ptr<Q, mem_mgr2>& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}

		template <class Q, typename mem_mgr2>
		shared_ptr& operator=(const weak_ptr<Q, mem_mgr2>& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}
	};

	// comparison operators for shared_ptr (cross-type)
	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator==(const shared_ptr<T, mem_mgr1>& lhs, const shared_ptr<Q, mem_mgr2>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator!=(const shared_ptr<T, mem_mgr1>& lhs, const shared_ptr<Q, mem_mgr2>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator<=(const shared_ptr<T, mem_mgr1>& lhs, const shared_ptr<Q, mem_mgr2>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator>(const shared_ptr<T, mem_mgr1>& lhs, const shared_ptr<Q, mem_mgr2>& rhs)
	{
		return rhs < lhs;
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator>=(const shared_ptr<T, mem_mgr1>& lhs, const shared_ptr<Q, mem_mgr2>& rhs)
	{
		return !(lhs < rhs);
	}

	// comparison with nullptr/0
	template <class T, typename mem_mgr>
	bool operator==(const shared_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator==(int /*null*/, const shared_ptr<T, mem_mgr>& rhs)
	{
		return rhs.get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(const shared_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.get() != SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(int /*null*/, const shared_ptr<T, mem_mgr>& rhs)
	{
		return rhs.get() != SMART_PTR_NULLPTR;
	}

#if SMART_PTR_SUPPORT_MOVE
	template <class T, typename mem_mgr>
	bool operator==(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() == nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator==(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr == rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator!=(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() != nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator!=(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr != rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator<(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() < nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator<(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr < rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator<=(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() <= nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator<=(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr <= rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator>(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() > nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator>(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr > rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator>=(const shared_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() >= nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator>=(std::nullptr_t, const shared_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr >= rhs.get();
	}
#endif

	// swap function
	template <class T, typename mem_mgr>
	void swap(shared_ptr<T, mem_mgr>& lhs, shared_ptr<T, mem_mgr>& rhs)
	{
		lhs.swap(rhs);
	}

	// ----------------------------------------------------------------
	// pointer casts (STL style) — share ref_count to avoid double-free
	//
	// IMPORTANT — virtual destructor requirement:
	//   Unlike std::shared_ptr, the deleter (mem_mgr) is NOT type-erased.
	//   The cast result carries mem_mgr::rebind<T>::other, so when the
	//   last strong reference is released the object is destroyed via
	//   "delete static_cast<T*>(ptr)".
	//
	//   When casting from a derived type to a base type, e.g.:
	//       shared_ptr<Base> b = static_pointer_cast<Base>(spDerived);
	//   the deleter becomes std_mem_mgr<Base>, which calls
	//   "delete (Base*)ptr".  This is well-defined ONLY IF Base has a
	//   virtual destructor.  If Base is non-polymorphic the behavior
	//   is undefined (partial destruction / memory leak).
	//
	//   Rule: always declare a virtual destructor on any base class
	//   that will be used as a cast target, e.g.:
	//       struct Base { virtual ~Base() {} };
	//
	//   Same-type casts (T == Q) and casts to more-derived types are
	//   always safe regardless of virtual destructor presence.
	// ----------------------------------------------------------------
	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> static_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		shared_ptr<T, typename mem_mgr::template rebind<T>::other> result;
		result.m_counter = sp.m_counter;
		result.m_ptr = static_cast<T*>(sp.get());
		if (result.m_counter)
		{
			result.m_counter->inc_ref();
		}
		return result;
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> dynamic_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		T* p = dynamic_cast<T*>(sp.get());
		if (p)
		{
			shared_ptr<T, typename mem_mgr::template rebind<T>::other> result;
			result.m_counter = sp.m_counter;
			result.m_ptr = p;
			if (result.m_counter)
			{
				result.m_counter->inc_ref();
			}
			return result;
		}
		return shared_ptr<T, typename mem_mgr::template rebind<T>::other>();
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> const_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		shared_ptr<T, typename mem_mgr::template rebind<T>::other> result;
		result.m_counter = sp.m_counter;
		result.m_ptr = const_cast<T*>(sp.get());
		if (result.m_counter)
		{
			result.m_counter->inc_ref();
		}
		return result;
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, typename mem_mgr::template rebind<T>::other> reinterpret_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		shared_ptr<T, typename mem_mgr::template rebind<T>::other> result;
		result.m_counter = sp.m_counter;
		result.m_ptr = reinterpret_cast<T*>(sp.get());
		if (result.m_counter)
		{
			result.m_counter->inc_ref();
		}
		return result;
	}

	template <class T, typename mem_mgr = std_mem_mgr<T> >
	class weak_ptr : public base_ptr<T, false, mem_mgr>
	{
		typedef base_ptr<T, false, mem_mgr> baseClass;

	private:
		// operator==/!= compare by control-block identity, which reads the protected
		// members.
		template <class TT, typename mm>
		friend bool operator==(const weak_ptr<TT, mm>& lhs, const weak_ptr<TT, mm>& rhs);
		template <class TT, typename mm>
		friend bool operator!=(const weak_ptr<TT, mm>& lhs, const weak_ptr<TT, mm>& rhs);

	public:
		// construct empty weak_ptr object
		weak_ptr()
		{
		}

		// construct weak_ptr object for resource owned by rhs
		template <class Q, typename mem_mgr2>
		weak_ptr(const shared_ptr<Q, mem_mgr2>& rhs) : baseClass(rhs)
		{
		}

		// construct weak_ptr object for resource pointed to by rhs
		weak_ptr(const weak_ptr& rhs) : baseClass(rhs)
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		// construct weak_ptr object for resource pointed to by rhs
		template <class Q, typename mem_mgr2>
		weak_ptr(const weak_ptr<Q, mem_mgr2>& rhs) : baseClass(rhs)
		{
		}
#endif

#if SMART_PTR_SUPPORT_MOVE
		// Move constructor
		weak_ptr(weak_ptr&& rhs) SMART_PTR_NOEXCEPT : baseClass(0)
		{
			this->m_counter = rhs.m_counter;
			this->m_ptr = rhs.m_ptr;
			rhs.m_counter = 0;
			rhs.m_ptr = 0;
		}

		// Move assignment
		weak_ptr& operator=(weak_ptr&& rhs) SMART_PTR_NOEXCEPT
		{
			if (this != &rhs)
			{
				this->release();
				this->m_counter = rhs.m_counter;
				this->m_ptr = rhs.m_ptr;
				rhs.m_counter = 0;
				rhs.m_ptr = 0;
			}
			return *this;
		}

		// Delegates to base_ptr, which holds the cross-type access rights.
		template <class Q, typename mem_mgr2>
		weak_ptr(weak_ptr<Q, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT
			: baseClass(static_cast<base_ptr<Q, false, mem_mgr2>&&>(rhs))
		{
		}

		template <class Q, typename mem_mgr2>
		weak_ptr& operator=(weak_ptr<Q, mem_mgr2>&& rhs) SMART_PTR_NOEXCEPT
		{
			baseClass::operator=(static_cast<base_ptr<Q, false, mem_mgr2>&&>(rhs));
			return *this;
		}
#endif

		~weak_ptr()
		{
		}

		weak_ptr& operator=(const weak_ptr& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q, typename mem_mgr2>
		weak_ptr& operator=(const weak_ptr<Q, mem_mgr2>& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}
#endif

		template <class Q, typename mem_mgr2>
		weak_ptr& operator=(const shared_ptr<Q, mem_mgr2>& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}

		// return true if resource no longer exists
		bool expired() const
		{
			return baseClass::m_counter ? baseClass::m_counter->expired() : true;
		}

		// convert to shared_ptr
		shared_ptr<T, mem_mgr> lock() const
		{
			return shared_ptr<T, mem_mgr>(*this);
		}

		// owner_before for use in ordered containers
		template <class Q, typename mem_mgr2>
		bool owner_before(const shared_ptr<Q, mem_mgr2>& other) const
		{
			return baseClass::m_counter < other.m_counter;
		}

		template <class Q, typename mem_mgr2>
		bool owner_before(const weak_ptr<Q, mem_mgr2>& other) const
		{
			return baseClass::m_counter < other.m_counter;
		}

	private:
		operator T* () const SMART_PTR_NOEXCEPT;
		T& operator*() const SMART_PTR_NOEXCEPT;
		T* operator->() const SMART_PTR_NOEXCEPT;
		T* get() const SMART_PTR_NOEXCEPT;
	};

	// Comparison operators for weak_ptr
	// Note: Compare control block and pointer directly for thread safety and correctness.
	// This ensures weak_ptrs from the same source are considered equal, even if expired.
	template <class T, typename mem_mgr>
	bool operator==(const weak_ptr<T, mem_mgr>& lhs, const weak_ptr<T, mem_mgr>& rhs)
	{
		return lhs.m_counter == rhs.m_counter && lhs.m_ptr == rhs.m_ptr;
	}

	template <class T, typename mem_mgr>
	bool operator!=(const weak_ptr<T, mem_mgr>& lhs, const weak_ptr<T, mem_mgr>& rhs)
	{
		return !(lhs == rhs);
	}

	// Comparison with nullptr
	template <class T, typename mem_mgr>
	bool operator==(const weak_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.lock().get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator==(int /*null*/, const weak_ptr<T, mem_mgr>& rhs)
	{
		return rhs.lock().get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(const weak_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.lock().get() != SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(int /*null*/, const weak_ptr<T, mem_mgr>& rhs)
	{
		return rhs.lock().get() != SMART_PTR_NULLPTR;
	}

#if SMART_PTR_SUPPORT_MOVE
	template <class T, typename mem_mgr>
	bool operator==(const weak_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.lock().get() == nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator==(std::nullptr_t, const weak_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr == rhs.lock().get();
	}

	template <class T, typename mem_mgr>
	bool operator!=(const weak_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.lock().get() != nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator!=(std::nullptr_t, const weak_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr != rhs.lock().get();
	}
#endif

	// swap for weak_ptr
	template <class T, typename mem_mgr>
	void swap(weak_ptr<T, mem_mgr>& lhs, weak_ptr<T, mem_mgr>& rhs)
	{
		lhs.swap(rhs);
	}

	// default_delete for unique_ptr
	template <class T>
	struct default_delete
	{
		void operator()(T* p) const { delete p; }
	};

	template <class T>
	struct default_delete<T[]>
	{
		void operator()(T* p) const { delete[] p; }
	};

	template <class T, typename mem_mgr = std_mem_mgr<T> >
	class unique_ptr
	{
	public:
		explicit unique_ptr(T* p = 0) : m_ptr(p)
		{
		}

		~unique_ptr()
		{
			do_delete();
		}

#if SMART_PTR_SUPPORT_MOVE
		// Move constructor
		unique_ptr(unique_ptr&& other) SMART_PTR_NOEXCEPT : m_ptr(other.m_ptr)
		{
			other.m_ptr = 0;
		}

		// Move assignment
		unique_ptr& operator=(unique_ptr&& other) SMART_PTR_NOEXCEPT
		{
			if (this != &other)
			{
				do_delete();
				m_ptr = other.m_ptr;
				other.m_ptr = 0;
			}
			return *this;
		}
#endif

		T& operator*() const SMART_PTR_NOEXCEPT { return *m_ptr; }

#if __cplusplus >= 201103L || _MSC_VER >= 1900
		// C++11: explicit bool conversion for conditional statements
		SMART_PTR_EXPLICIT_BOOL() const SMART_PTR_NOEXCEPT { return m_ptr != 0; }
#else
		// C++03: safe bool idiom to support if(sp) without allowing int x = sp;
		typedef T* (unique_ptr::*unspecified_bool_type)() const;
		operator unspecified_bool_type() const SMART_PTR_NOEXCEPT
		{
			return m_ptr ? &unique_ptr::get : 0;
		}
#endif

		// Negation operator: if (!up) { ... }
		bool operator!() const SMART_PTR_NOEXCEPT
		{
			return !m_ptr;
		}

#if defined(WIN32) || defined(_WIN32)
		_NoAddRefReleaseOnComPtr<T>* operator->() const SMART_PTR_NOEXCEPT
		{
			return (_NoAddRefReleaseOnComPtr<T>*)m_ptr;
		}
#else
		T* operator->() const SMART_PTR_NOEXCEPT
		{
			return m_ptr;
		}
#endif // defined(WIN32) || defined(_WIN32)
		T* get() const SMART_PTR_NOEXCEPT
		{
			return m_ptr;
		}

		bool unique() const SMART_PTR_NOEXCEPT
		{
			return true;
		}

		void reset(T* p = 0)
		{
			do_delete();
			m_ptr = p;
		}

		T* release()
		{
			T* tmp = m_ptr;
			m_ptr = 0;
			return tmp;
		}

		int use_count(void) const
		{
			return 1;
		}

		// swap pointers (same-type only: a different mem_mgr would call the
		// wrong deleter on destruction)
		void swap(unique_ptr& rhs)
		{
			private_swap(m_ptr, rhs.m_ptr);
		}

		T* m_ptr;
	protected:

		template <typename TP1, typename TP2>
		static void private_swap(TP1& obj1, TP2& obj2)
		{
			TP1 tmp = obj1;
			obj1 = static_cast<TP1>(obj2);
			obj2 = static_cast<TP2>(tmp);
		}

		void do_delete(void)
		{
			if (m_ptr)
			{
				mem_mgr::deallocate(m_ptr);
				m_ptr = 0;
			}
		}
#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q, typename mem_mgr2>
		friend class unique_ptr;
#endif
		//noncopyable
	private:
		template <class Q, typename mem_mgr2>
		unique_ptr(const unique_ptr<Q, mem_mgr2>& rhs);
#if !defined(_MSC_VER) || _MSC_VER >= 1300
		unique_ptr& operator=(const unique_ptr& rhs);
#endif
		template <class Q, typename mem_mgr2>
		unique_ptr& operator=(const unique_ptr<Q, mem_mgr2>& rhs);

		template <class Q, typename mem_mgr2>
		void acquire(const unique_ptr<Q, mem_mgr2>& rhs) SMART_PTR_NOEXCEPT;

		template <class Q, typename mem_mgr2>
		void reset(const unique_ptr<Q, mem_mgr2>& rhs);
	};

	// comparison operators for unique_ptr (cross-type)
	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator<(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		// test if left pointer < right pointer
		return lhs.get() < rhs.get();
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator==(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator!=(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		return !(lhs == rhs);
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator<=(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		return !(rhs < lhs);
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator>(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		return rhs < lhs;
	}

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator>=(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		return !(lhs < rhs);
	}

	// comparison with nullptr
	template <class T, typename mem_mgr>
	bool operator==(const unique_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator==(int /*null*/, const unique_ptr<T, mem_mgr>& rhs)
	{
		return rhs.get() == SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(const unique_ptr<T, mem_mgr>& lhs, int /*null*/)
	{
		return lhs.get() != SMART_PTR_NULLPTR;
	}

	template <class T, typename mem_mgr>
	bool operator!=(int /*null*/, const unique_ptr<T, mem_mgr>& rhs)
	{
		return rhs.get() != SMART_PTR_NULLPTR;
	}

#if SMART_PTR_SUPPORT_MOVE
	template <class T, typename mem_mgr>
	bool operator==(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() == nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator==(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr == rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator!=(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() != nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator!=(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr != rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator<(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() < nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator<(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr < rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator<=(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() <= nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator<=(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr <= rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator>(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() > nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator>(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr > rhs.get();
	}

	template <class T, typename mem_mgr>
	bool operator>=(const unique_ptr<T, mem_mgr>& lhs, std::nullptr_t) SMART_PTR_NOEXCEPT
	{
		return lhs.get() >= nullptr;
	}

	template <class T, typename mem_mgr>
	bool operator>=(std::nullptr_t, const unique_ptr<T, mem_mgr>& rhs) SMART_PTR_NOEXCEPT
	{
		return nullptr >= rhs.get();
	}
#endif

	// swap for unique_ptr
	template <class T, typename mem_mgr>
	void swap(unique_ptr<T, mem_mgr>& lhs, unique_ptr<T, mem_mgr>& rhs)
	{
		lhs.swap(rhs);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//   make_shared/make_unique (STL style)
	//

	template <typename T>
	shared_ptr<T> make_shared()
	{
		return shared_ptr<T>(new T());
	}

	template <typename T, typename A1>
	shared_ptr<T> make_shared(A1 const& a1)
	{
		return shared_ptr<T>(new T(a1));
	}

	template <typename T, typename A1, typename A2>
	shared_ptr<T> make_shared(A1 const& a1, A2 const& a2)
	{
		return shared_ptr<T>(new T(a1, a2));
	}

	template <typename T, typename A1, typename A2, typename A3>
	shared_ptr<T> make_shared(A1 const& a1, A2 const& a2, A3 const& a3)
	{
		return shared_ptr<T>(new T(a1, a2, a3));
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4>
	shared_ptr<T> make_shared(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4));
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	shared_ptr<T> make_shared(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5));
	}

	template <typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	shared_ptr<T> make_shared(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
	{
		return shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6));
	}

	// make_unique (C++11 can use variadic templates, C++03 uses overloads)
	template <typename T>
	unique_ptr<T> make_unique()
	{
		return unique_ptr<T>(new T());
	}

	template <typename T, typename A1>
	unique_ptr<T> make_unique(A1 const& a1)
	{
		return unique_ptr<T>(new T(a1));
	}

	template <typename T, typename A1, typename A2>
	unique_ptr<T> make_unique(A1 const& a1, A2 const& a2)
	{
		return unique_ptr<T>(new T(a1, a2));
	}

	template <typename T, typename A1, typename A2, typename A3>
	unique_ptr<T> make_unique(A1 const& a1, A2 const& a2, A3 const& a3)
	{
		return unique_ptr<T>(new T(a1, a2, a3));
	}

	//////////////////////////////////////////////////////////////////////////
	// COM pointer support
	//

	template <typename T>
	class com_mem_mgr
	{
	public:
		// rebind<U>::other yields com_mem_mgr<U>; see std_mem_mgr::rebind.
		template <typename U>
		struct rebind
		{
			typedef com_mem_mgr<U> other;
		};

		static void deallocate(T* p) { p->Release(); }
		static T* allocate(T* p)
		{
			p->AddRef();
			return p;
		} // we must hold the AddRef-ed pointer
	};

	template <typename T>
	shared_ptr<T, com_mem_mgr<T> > make_com_shared_ptr(T* rawPtr)
	{
		return shared_ptr<T, com_mem_mgr<T> >(rawPtr);
	}

	//////////////////////////////////////////////////////////////////////////
	// auto-released array support
	//

	template <typename T>
	class array_mem_mgr
	{
	public:
		// rebind<U>::other yields array_mem_mgr<U>; see std_mem_mgr::rebind.
		template <typename U>
		struct rebind
		{
			typedef array_mem_mgr<U> other;
		};

		static void deallocate(T* p) { delete[] p; }
		static T* allocate(int n) { return new T[n]; }
	};

	template <class T, typename mem_mgr = array_mem_mgr<T> >
	class shared_array : public base_ptr<T, true, mem_mgr>
	{
		typedef base_ptr<T, true, mem_mgr> baseClass;

	public:
		explicit shared_array(T* p = 0) : baseClass(p)
		{
		}

		shared_array(const shared_array& rhs) : baseClass(rhs)
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q>
		shared_array(const shared_array<Q, mem_mgr>& rhs) : baseClass(rhs)
		{
		}
#endif

		~shared_array()
		{
		}

		const T& operator[](int i) const
		{
			return baseClass::get()[i];
		}

		T& operator[](int i)
		{
			return baseClass::get()[i];
		}

		shared_array& operator=(const shared_array& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q>
		shared_array& operator=(const shared_array<Q, mem_mgr>& rhs)
		{
			baseClass::operator=(rhs);
			return *this;
		}
#endif

	private:
		T& operator*() const SMART_PTR_NOEXCEPT;
		T* operator->() const SMART_PTR_NOEXCEPT;
	};

	//////////////////////////////////////////////////////////////////////////
	// define macros

#ifndef EMPTY_NAME_SPACE
#define EMPTY_NAME_SPACE
#endif // EMPTY_NAME_SPACE

// defining COM smart pointer type
#ifndef DEFINE_COM_STRONG_PTR
#define DEFINE_COM_STRONG_PTR(NAME_SPACE_T, TYPE) \
	typedef smart_ptr::shared_ptr<NAME_SPACE_T::TYPE, smart_ptr::com_mem_mgr<NAME_SPACE_T::TYPE> > TYPE##ComPtr;
#endif // DEFINE_COM_STRONG_PTR

// defining standard smart pointer type
#ifndef DEFINE_STD_STRONG_PTR
#define DEFINE_STD_STRONG_PTR(NAME_SPACE_T, TYPE) \
	typedef smart_ptr::shared_ptr<NAME_SPACE_T::TYPE, smart_ptr::std_mem_mgr<NAME_SPACE_T::TYPE> > TYPE##StdPtr;
#endif // DEFINE_STD_STRONG_PTR

// defining array style smart pointer type
#ifndef DEFINE_ARR_STRONG_PTR
#define DEFINE_ARR_STRONG_PTR(NAME_SPACE_T, TYPE) \
	typedef smart_ptr::shared_array<NAME_SPACE_T::TYPE, smart_ptr::array_mem_mgr<NAME_SPACE_T::TYPE> > TYPE##ArrPtr;
#endif // DEFINE_ARR_STRONG_PTR

} // namespace smart_ptr

#endif // __SMART_PTR_H__
