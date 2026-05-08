#ifndef _STK_RBTREE_CXX_H
#define _STK_RBTREE_CXX_H

#include <cstddef>

namespace stk {

		enum class Color {
			RED,
			BLACK
		};

		template<typename T>
		struct rbnode
		{
			T data_;
			Color color_;
			rbnode* left_;
			rbnode* right_;
			rbnode* parent_;

			rbnode(const T& data) :
				data_(data),
				color_(Color::RED),
				left_(nullptr),
				right_(nullptr),
				parent_(nullptr) {}
		};

		template<typename T>
		class rbtree
		{
		public:
			rbtree();
			~rbtree();

			void insert(const T& value);
			void remove(const T& value);
			bool find(const T& value) const;
			bool isEmpty() const;
			void inorderPrint() const;

		private:
			void leftRotate(rbnode<T>* x);
			void rightRotate(rbnode<T>* y);
			void fixInsert(rbnode<T>* z);
			void fixDelete(rbnode<T>* x);
			void transplant(rbnode<T>* u, rbnode<T>* v);
			rbnode<T>* minimum(rbnode<T>* x) const;
			rbnode<T>* maximum(rbnode<T>* x) const;
			rbnode<T>* searchNode(const T& value) const;
			void clearHelper(rbnode<T>* node);
			void inorderHelper(rbnode<T>* node) const;

		private:
			rbnode<T>* root_;
			rbnode<T>* nil_;

		};

} 

#include "stk_rbtree.tcc"

#endif 
