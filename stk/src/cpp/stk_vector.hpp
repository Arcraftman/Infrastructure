#ifndef _STK_VECTOR_CXX_H
#define _STK_VECTOR_CXX_H

#pragma once

#include <initializer_list>
#include <cstddef>

namespace stk {

		template<typename T>
		class vector
		{
		public:
			vector();
			explicit vector(size_t _size);
			vector(std::initializer_list<T> _list);

			vector(const vector& _other);
			vector(vector&& _other) noexcept;

			~vector();

			vector& operator=(const vector& _other);
			vector& operator=(vector&& _other) noexcept;

			T operator[](size_t _index) const;
			T& operator[](size_t _index);

			vector operator+(const vector& _other) const;
			vector operator-(const vector& _other) const;
			vector operator*(T _scalar) const;

			template<typename U>
			friend vector<U> operator*(U _scalar, const vector<U>& _vec);

			vector& operator+=(const vector& _other);
			vector& operator-=(const vector& _other);
			vector& operator*=(T _scalar);

			size_t size() const;
			void resize(size_t _size);
			void normalize();
			T norm() const;
			T dot(const vector& _other) const;

			void fill(T _value);
			const T* data() const;

			static vector zeros(size_t _size);
			static vector ones(size_t _size);
			static vector random(size_t _size, T _min = 0.0, T _max = 1.0);

		private:
			T* data_;
			size_t size_;
		};

		template<typename T>
		inline vector<T> operator*(T _scalar, const vector<T>& _vec)
		{
			return _vec * _scalar;
		}
}

#include "stk_vector.tcc"

#endif
