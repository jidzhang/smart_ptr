/*
 * smart_ptr - simple reference counted pointer.
 *
 * Copyright (c) 2013, oddman
 * https://github.com/oddman2017/SmartPointer/
 *
 * The is a non-intrusive implementation that allocates an additional
 * int and pointer for every counted object.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

// Platform-specific atomic operations
#if defined(WIN32) || defined(_WIN32)
	// Windows: Use Interlocked API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#elif defined(__GNUC__) || defined(__clang__)
	// GCC/Clang: Use __sync built-in (available since GCC 4.1)
	// These provide full memory barrier and are available on both GCC and Clang

#else
	// Unknown platform: atomic operations NOT available, fallback to non-thread-safe
	#define SMART_PTR_NO_ATOMIC 1
#endif

#if __cplusplus >= 201103L || _MSC_VER >= 1900
#define SMART_PTR_SUPPORT_MOVE 1
#include <algorithm> // for std::swap in C++11
#else
#define SMART_PTR_SUPPORT_MOVE 0
#endif

namespace smart_ptr
{
	// Reference count with thread-safe increment/decrement
	class ref_count
	{
	public:
		ref_count() : m_strong_ref_count(1), m_weak_ref_count(0)
		{
		}

		~ref_count()
		{
		}

		// increment use count (thread-safe)
		int inc_ref()
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedIncrement(&m_strong_ref_count));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_add_and_fetch(&m_strong_ref_count, 1);
#else
			// Non-thread-safe fallback
			return ++m_strong_ref_count;
#endif
		}

		// increment weak reference count (thread-safe)
		int inc_weak_ref()
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedIncrement(&m_weak_ref_count));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_add_and_fetch(&m_weak_ref_count, 1);
#else
			// Non-thread-safe fallback
			return ++m_weak_ref_count;
#endif
		}

		// decrement use count (thread-safe)
		int dec_ref()
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedDecrement(&m_strong_ref_count));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_sub_and_fetch(&m_strong_ref_count, 1);
#else
			// Non-thread-safe fallback
			return --m_strong_ref_count;
#endif
		}

		// decrement weak reference count (thread-safe)
		int dec_weak_ref()
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedDecrement(&m_weak_ref_count));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_sub_and_fetch(&m_weak_ref_count, 1);
#else
			// Non-thread-safe fallback
			return --m_weak_ref_count;
#endif
		}

		// Atomically try to increment strong ref count if it's not zero
		// Returns true if increment succeeded (object is alive), false otherwise
		bool try_inc_ref()
		{
#if defined(WIN32) || defined(_WIN32)
			// Use InterlockedCompareExchange to atomically check-and-increment
			volatile LONG* ptr = &m_strong_ref_count;
			LONG current = *ptr;
			while (current != 0) {
				LONG new_val = current + 1;
				LONG old_val = InterlockedCompareExchange(ptr, new_val, current);
				if (old_val == current) {
					return true;  // Successfully incremented
				}
				current = old_val;  // Retry with new value
			}
			return false;  // Count was 0, object is being destroyed
#elif defined(__GNUC__) || defined(__clang__)
			// Use __sync builtins for atomic compare-and-swap
			volatile int* ptr = &m_strong_ref_count;
			int current = *ptr;
			while (current != 0) {
				int new_val = current + 1;
				int old_val = __sync_val_compare_and_swap(ptr, current, new_val);
				if (old_val == current) {
					return true;
				}
				current = old_val;
			}
			return false;
#else
			// Non-thread-safe fallback
			if (m_strong_ref_count > 0) {
				++m_strong_ref_count;
				return true;
			}
			return false;
#endif
		}

		// return use count (atomic read)
		int get_ref_count() const
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedCompareExchange(
				const_cast<volatile LONG*>(&m_strong_ref_count), 0, 0));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_fetch_and_add(const_cast<volatile int*>(&m_strong_ref_count), 0);
#else
			return m_strong_ref_count;
#endif
		}

		// return true if _Uses == 0
		bool expired() const
		{
			return (get_ref_count() == 0);
		}

		// return weak reference count (atomic read)
		int get_weak_ref_count() const
		{
#if defined(WIN32) || defined(_WIN32)
			return static_cast<int>(InterlockedCompareExchange(
				const_cast<volatile LONG*>(&m_weak_ref_count), 0, 0));
#elif defined(__GNUC__) || defined(__clang__)
			return __sync_fetch_and_add(const_cast<volatile int*>(&m_weak_ref_count), 0);
#else
			return m_weak_ref_count;
#endif
		}

	private:
#if defined(WIN32) || defined(_WIN32)
		volatile LONG m_strong_ref_count;
		volatile LONG m_weak_ref_count;
#else
		// Use int on non-Windows platforms (GCC/Clang __sync built-ins work with int)
		volatile int m_strong_ref_count;
		volatile int m_weak_ref_count;
#endif
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

	// base class for shared_ptr and weak_ptr
	template <class T, bool is_strong, typename mem_mgr>
	class base_ptr
	{
	public:
		explicit base_ptr(T* p = 0) : m_counter(0), m_ptr(p)
		{
			if (m_ptr)
			{
				if (is_strong)
				{
					// allocate a new ref_count
					m_counter = new ref_count;
				}
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

		virtual ~base_ptr()
		{
			release();
		}

		// Note: Implicit conversion to T* is intentionally removed for safety.
		// Use .get() to obtain the raw pointer explicitly.
		T& operator*() const throw() { return *m_ptr; }

#if SMART_PTR_SUPPORT_MOVE
		// C++11: explicit bool conversion for conditional statements
		explicit operator bool() const noexcept { return m_ptr != 0; }
#else
		// C++03: safe bool idiom to support if(sp) without allowing int x = sp;
		typedef T* (base_ptr::*unspecified_bool_type)() const;
		operator unspecified_bool_type() const { return m_ptr ? &base_ptr::get : 0; }
#endif

#if defined(WIN32) || defined(_WIN32)
		_NoAddRefReleaseOnComPtr<T>* operator->() const throw()
		{
			return (_NoAddRefReleaseOnComPtr<T>*)m_ptr;
		}
#else
		T* operator->() const throw()
		{
			return m_ptr;
		}
#endif // defined(WIN32) || defined(_WIN32)
		T* get() const throw()
		{
			return m_ptr;
		}

		bool unique() const throw()
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

		// swap pointers
		template <class Q, bool b, typename mem_mgr2>
		void swap(base_ptr<Q, b, mem_mgr2>& rhs)
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

		ref_count* m_counter;
		T* m_ptr;
	protected:

		template <typename TP1, typename TP2>
		static void private_swap(TP1& obj1, TP2& obj2)
		{
			TP1 tmp = obj1;
			obj1 = static_cast<TP1>(obj2);
			obj2 = static_cast<TP2>(tmp);
		}

		template <class Q, bool b, typename mem_mgr2>
		void acquire(const base_ptr<Q, b, mem_mgr2>& rhs) throw()
		{
			if (rhs.m_counter)
			{
				if (is_strong)
				{
					// Try to atomically increment strong ref count if not zero
					// This prevents TOCTOU race condition
					if (!rhs.m_counter->try_inc_ref())
					{
						return;  // Object already expired, don't acquire
					}
					m_counter = rhs.m_counter;
				}
				else
				{
					// Weak ref increment is always safe (ref_count stays alive
					// as long as there's at least one weak_ptr)
					m_counter = rhs.m_counter;
					m_counter->inc_weak_ref();
				}
				m_ptr = static_cast<T*>(rhs.m_ptr);
			}
		}

		// decrement the count, delete if it is 0
		void release(void)
		{
			if (m_counter)
			{
				if (is_strong)
				{
					if (0 == m_counter->dec_ref())
					{
						mem_mgr::deallocate(m_ptr);
						m_ptr = 0;
					}
				}
				else
				{
					m_counter->dec_weak_ref();
				}
				if (0 == m_counter->get_ref_count() && 0 == m_counter->get_weak_ref_count())
				{
					delete m_counter;
					m_counter = 0;
				}
			}
			if (m_ptr)
			{
				m_ptr = 0;
			}
		}
#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template <class Q, bool b, typename mem_mgr2>
		friend class base_ptr;
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

	// comparison operators for shared_ptr
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

	// comparison with nullptr
	template <class T, typename mem_mgr>
	bool operator==(const shared_ptr<T, mem_mgr>& lhs, int null)
	{
		return lhs.get() == 0;
	}

	template <class T, typename mem_mgr>
	bool operator!=(const shared_ptr<T, mem_mgr>& lhs, int null)
	{
		return lhs.get() != 0;
	}

	// swap function
	template <class T, typename mem_mgr>
	void swap(shared_ptr<T, mem_mgr>& lhs, shared_ptr<T, mem_mgr>& rhs)
	{
		lhs.swap(rhs);
	}

	// pointer casts
	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, mem_mgr> static_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		return shared_ptr<T, mem_mgr>(static_cast<T*>(sp.get()));
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, mem_mgr> dynamic_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		T* p = dynamic_cast<T*>(sp.get());
		if (p)
			return shared_ptr<T, mem_mgr>(p);
		return shared_ptr<T, mem_mgr>();
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, mem_mgr> const_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		return shared_ptr<T, mem_mgr>(const_cast<T*>(sp.get()));
	}

	template <class T, class Q, typename mem_mgr>
	shared_ptr<T, mem_mgr> reinterpret_pointer_cast(const shared_ptr<Q, mem_mgr>& sp)
	{
		return shared_ptr<T, mem_mgr>(reinterpret_cast<T*>(sp.get()));
	}

	template <class T, typename mem_mgr = std_mem_mgr<T> >
	class weak_ptr : public base_ptr<T, false, mem_mgr>
	{
		typedef base_ptr<T, false, mem_mgr> baseClass;

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
		operator T* () const throw();
		T& operator*() const throw();
		T* operator->() const throw();
		T* get() const throw();
	};

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
		// move constructor
		unique_ptr(unique_ptr&& other) noexcept : m_ptr(other.m_ptr)
		{
			other.m_ptr = 0;
		}

		// move assignment
		unique_ptr& operator=(unique_ptr&& other) noexcept
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

		// Note: Implicit conversion to T* is intentionally removed for safety.
		// Use .get() to obtain the raw pointer explicitly.
		T& operator*() const throw() { return *m_ptr; }

#if SMART_PTR_SUPPORT_MOVE
		// C++11: explicit bool conversion for conditional statements
		explicit operator bool() const noexcept { return m_ptr != 0; }
#else
		// C++03: safe bool idiom to support if(sp) without allowing int x = sp;
		typedef T* (unique_ptr::*unspecified_bool_type)() const;
		operator unspecified_bool_type() const { return m_ptr ? &unique_ptr::get : 0; }
#endif

#if defined(WIN32) || defined(_WIN32)
		_NoAddRefReleaseOnComPtr<T>* operator->() const throw()
		{
			return (_NoAddRefReleaseOnComPtr<T>*)m_ptr;
		}
#else
		T* operator->() const throw()
		{
			return m_ptr;
		}
#endif // defined(WIN32) || defined(_WIN32)
		T* get() const throw()
		{
			return m_ptr;
		}

		bool unique() const throw()
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

		// swap pointers
		template <class Q, typename mem_mgr2>
		void swap(unique_ptr<Q, mem_mgr2>& rhs)
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
		// noncopyable
	private:
	template <class Q, typename mem_mgr2>
		unique_ptr(const unique_ptr<Q, mem_mgr2>& rhs);
#if !defined(_MSC_VER) || _MSC_VER >= 1300
		unique_ptr& operator=(const unique_ptr& rhs);
#endif
		template <class Q, typename mem_mgr2>
		unique_ptr& operator=(const unique_ptr<Q, mem_mgr2>& rhs);

		template <class Q, typename mem_mgr2>
		void acquire(const unique_ptr<Q, mem_mgr2>& rhs) throw();

		template <class Q, typename mem_mgr2>
		void reset(const unique_ptr<Q, mem_mgr2>& rhs);
	};

	template <class T, class Q, typename mem_mgr1, typename mem_mgr2>
	bool operator<(const unique_ptr<T, mem_mgr1>& lhs, const unique_ptr<Q, mem_mgr2>& rhs)
	{
		// test if left pointer < right pointer
		return lhs.get() < rhs.get();
	}

	// comparison operators for unique_ptr
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

	// swap for unique_ptr
	template <class T, typename mem_mgr>
	void swap(unique_ptr<T, mem_mgr>& lhs, unique_ptr<T, mem_mgr>& rhs)
	{
		lhs.swap(rhs);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//   function make_shared_ptr group
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

	// make_unique - STL style
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
		T& operator*() const throw();
		T* operator->() const throw();
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
}; // namespace smart_ptr

#endif // __SMART_PTR_H__
