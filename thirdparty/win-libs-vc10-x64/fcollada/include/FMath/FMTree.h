/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/**
	@file FMTree.h
	The file contains the tree class, which improves on the standard C++ tree class.
 */

#ifndef _FM_TREE_H_
#define _FM_TREE_H_

#ifndef _FM_ALLOCATOR_H_
#include "FMath/FMAllocator.h"
#endif // _FM_ALLOCATOR_H_

namespace fm
{
	/** 
		A simple pair template.
		@ingroup FMath
	*/
	template <class _Kty, class _Ty>
	class pair
	{
	public:
		_Kty first; /**< The first element of the pair. */
		_Ty second; /**< The second element of the pair. */

		/** Constructor. */
		pair() : first(), second() {}

		/** Constructor.
			@param f A first element to copy.
			@param s A second element to copy. */
		pair(const _Kty& f, const _Ty& s) : first(f), second(s) {}

		/** Copy constructor.
			@param p A pair to clone. */
		pair(const pair& p) : first(p.first), second(p.second) {}

		/** Returns whether the two pairs are equal.
			@param p A second pair of the same type.
			@return Whether the two pairs are equal. */
		inline bool operator==(const pair& p) const { return p.first == first && p.second == second; }

		/** Returns whether the two pairs are not equal.
			@param p A second pair of the same type.
			@return Whether the two pairs are not equal. */
		inline bool operator!=(const pair& p) const { return p.first != first || p.second != second; }
	};

	/**
		An auto-balancing tree.
		Intentionally has an interface similar to the standard C++ tree class.
		It's implement should be very similar yet more lightweight.
		
		@ingroup FMath
	*/
	template <class KEY, class DATA>
	class tree
	{
	private:
		typedef fm::pair<KEY, DATA> pair;

		class node
		{
		public:
			node* left;
			node* right;
			node* parent;

			int32 weight;

			pair data;

		public:
			node() : left(NULL), right(NULL), parent(NULL), weight(0) {}

			void rotateLeft()
			{
				node** parentLink = (parent->left == this) ? &parent->left : &parent->right;

				// detach right
				node* oldRight = right;

				// detach right's left and attach to the parent's right.
				node* right_left = right->left;
				right = right_left;
				if (right_left != NULL) right_left->parent = this;

				// attach the right to the double parent.
				oldRight->left = this;

				// attach the parent on the right's left.
				oldRight->parent = parent;
				parent = oldRight;
				(*parentLink) = oldRight;

				// adjust the weights
				weight = weight - 1 - (oldRight->weight > 0 ? oldRight->weight : 0);
				oldRight->weight = oldRight->weight - (0 > -weight ? 0 : -weight) - 1;
			}

			void rotateRight()
			{
				node** parentLink = (parent->left == this) ? &parent->left : &parent->right;
				
				// detach left
				node* oldLeft = left;

				// detach left's right and attach to the parent's left.
				node* left_right = left->right;
				left = left_right;
				if (left_right != NULL) left_right->parent = this;

				// attach the parent on the left's right.
				oldLeft->right = this;

				// attach the left to the double parent.
				oldLeft->parent = parent;
				parent = oldLeft;
				(*parentLink) = oldLeft;

				// adjust the weights
				weight = weight + 1 + (0 > oldLeft->weight ? -oldLeft->weight : 0);
				oldLeft->weight = (oldLeft->weight + 1) + (0 > weight ? 0 : weight);
			}

#ifdef TREE_DEBUG
			intptr_t depth() const
			{
				intptr_t leftDepth = left != NULL ? left->depth() : 0;
				intptr_t rightDepth = right != NULL ? right->depth() : 0;
				return max(leftDepth, rightDepth) + 1;
			}

			void is_correct()
			{
				if (left != NULL) left->is_correct();
				if (right != NULL) right->is_correct();
				intptr_t leftDepth = left != NULL ? left->depth() : 0;
				intptr_t rightDepth = right != NULL ? right->depth() : 0;
				FUAssert(rightDepth - leftDepth == weight,);
				FUAssert(abs(weight) < 2,);
			}
#endif // TREE_DEBUG
		};

	public:
		class const_iterator;

		/**
			A tree element iterator.
			Similar to the basic STL iterator.
		*/
		class iterator
		{
		private:
			friend class tree;
			friend class const_iterator;
			node* currentNode;

		public:
			/** Empty constructor. */
			iterator() {}
			/** Constructor. @param n The tree node at which to start the iteration. */
			iterator(node* n) : currentNode(n) {}
			/** Copy operator. @param copy The tree iterator to copy. */
			iterator& operator=(const iterator& copy) { currentNode = copy.currentNode; return *this; }

			/** Retrieves whether this iterator points to the same node as the given iterator.
				@param other A second iterator.
				@return Whether the two iterators are pointing to the same node. */
			inline bool operator==(const iterator& other) const { return other.currentNode == currentNode; }
			inline bool operator==(const const_iterator& other) const; /**< See above. */

			/** Retrieves whether this iterator points to a different node that a given iterator.
				@param other A second iterator.
				@return Whether the two iterators are pointing at different nodes. */
			inline bool operator!=(const iterator& other) const { return other.currentNode != currentNode; }
			inline bool operator!=(const const_iterator& other) const;/**< See above. */

			/** Advances the iterator to the next ordered tree node.
				@return This iterator. */
			iterator& operator++()
			{
				// Go one right or one up.
				if (currentNode->right == NULL)
				{
					node* oldNode;
					do
					{
						oldNode = currentNode;

						// Go one up.
						// We control the root node, which is the only with parent == NULL.
						// if you crash here, you don't check for iterator == end() correctly.
						currentNode = currentNode->parent;
					}
					while (currentNode->right == oldNode && currentNode->parent != NULL);
				}
				else
				{
					// Go one right.
					currentNode = currentNode->right;

					// Go all the way left.
					while (currentNode->left != NULL) currentNode = currentNode->left;
				}
				return (*this);
			}

			/** Backtrack the iterator to the next ordered tree node.
				@return This iterator. */
			iterator& operator--()
			{
				// Go one left or one up.
				if (currentNode->left == NULL)
				{
					node* oldNode;
					do
					{
						oldNode = currentNode;

						// Go one up.
						// We control the root node, which is the only with parent == NULL.
						// if you crash here, you don't check for iterator == begin() correctly.
						currentNode = currentNode->parent;
					}
					while (currentNode->left == oldNode && currentNode->parent != NULL);
				}
				else
				{
					// Go one left.
					currentNode = currentNode->left;

					// Go all the way right.
					while (currentNode->right != NULL) currentNode = currentNode->right;
				}
				return (*this);
			}

			/** Retrieves the current tree node.
				@return The current tree node. */
			inline pair& operator*() { return currentNode->data; }
			inline pair* operator->() { return &currentNode->data; }  /**< See above. */
		};

		/**
			A tree constant-element iterator.
			Similar to the basic STL const_iterator.
		*/
		class const_iterator
		{
		private:
			friend class tree;
			friend class iterator;
			const node* currentNode;

		public:
			/** Empty constructor. */
			const_iterator() {}
			/** Copy constructor.
				@param copy The iterator to clone. */
			const_iterator(const iterator& copy) : currentNode(copy.currentNode) {}
			/** Constructor. @param n The tree node at which to start the iteration. */
			const_iterator(const node* n) : currentNode(n) {}
			/** Copy operator. @param copy The tree iterator to copy. */
			const_iterator& operator=(const iterator& copy) { currentNode = copy.currentNode; return *this; }
			const_iterator& operator=(const const_iterator& copy) { currentNode = copy.currentNode; return *this; } /**< See above. */

			/** Retrieves whether this iterator points to the same node as the given iterator.
				@param other A second iterator.
				@return Whether the two iterators are pointing to the same node. */
			inline bool operator==(const iterator& other) const { return other.currentNode == currentNode; }
			inline bool operator==(const const_iterator& other) const { return other.currentNode == currentNode; } /**< See above. */

			/** Retrieves whether this iterator points to a different node that a given iterator.
				@param other A second iterator.
				@return Whether the two iterators are pointing at different nodes. */
			inline bool operator!=(const iterator& other) const { return other.currentNode != currentNode; }
			inline bool operator!=(const const_iterator& other) const { return other.currentNode != currentNode; } /**< See above. */

			/** Advances the iterator to the next ordered tree node.
				@return This iterator. */
			const_iterator& operator++()
			{
				// Go one right or one up.
				if (currentNode->right == NULL)
				{
					const node* oldNode;
					do
					{
						oldNode = currentNode;

						// Go one up.
						// We control the root node, which is the only with parent == NULL.
						// if you crash here, you don't check for iterator == end() correctly.
						currentNode = currentNode->parent;
					}
					while (currentNode->right == oldNode && currentNode->parent != NULL);
				}
				else
				{
					// Go one right.
					currentNode = currentNode->right;

					// Go all the way left.
					while (currentNode->left != NULL) currentNode = currentNode->left;
				}
				return (*this);
			}

			/** Backtrack the iterator to the next ordered tree node.
				@return This iterator. */
			const_iterator& operator--()
			{
				// Go one left or one up.
				if (currentNode->left == NULL)
				{
					const node* oldNode;
					do
					{
						oldNode = currentNode;

						// Go one up.
						// We control the root node, which is the only with parent == NULL.
						// if you crash here, you don't check for iterator == end() correctly.
						currentNode = currentNode->parent;
					}
					while (currentNode->left == oldNode && currentNode->parent != NULL);
				}
				else
				{
					// Go one left.
					currentNode = currentNode->left;

					// Go all the way right.
					while (currentNode->right != NULL) currentNode = currentNode->right;
				}
				return (*this);
			}

			/** Retrieves the current tree node.
				@return The current tree node. */
			inline const pair& operator*() { return currentNode->data; }
			inline const pair* operator->() { return &currentNode->data; } /**< See above. */
		};

	private:
		node* root;
		size_t sized;

	public:
		/** Constructor. */
		tree() : root(NULL), sized(0)
		{
			root = (node*) fm::Allocate(sizeof(node));
			fm::Construct(root);
		}

		/** Destructor. */
		~tree()
		{
			clear();
			root->data.first.~KEY();
			root->data.second.~DATA();
			fm::Release(root);
			root = NULL;
		}

		/** Retrieves the first ordered element within the tree. 
			@return An iterator that points to the first tree element. */
		inline iterator begin() { iterator it(root); return (root->right == NULL) ? it : ++it; }
		inline const_iterator begin() const { const_iterator it(root); return (root->right == NULL) ? it : ++it; } /**< See above. */

		/** Retrieves an iterator that points just passed the last
			ordered element within the tree.
			@return An iterator just passed the last element in the tree. */
		inline iterator end() { return iterator(root); }
		inline const_iterator end() const { return const_iterator(root); } /**< See above. */

		/** Retrieves the last ordered element within the tree. 
			@return An iterator that points to the last tree element. */
		inline iterator last() { node* n = root; while (n->right != NULL) n = n->right; return iterator(n); }
		inline const_iterator last() const { const node* n = root; while (n->right != NULL) n = n->right; return const_iterator(n); } /**< See above. */

		/** Retrieves an existing data element using its key.
			@param key The key.
			@return An iterator that points to the existing data element.
				This iterator may be past the end of the tree, in the case
				where the key does not exists within the tree: do check the
				iterator against end(). */
		iterator find(const KEY& key)
		{
			node* out = root->right;
			while (out != NULL)
			{
				if (key < out->data.first) out = out->left;
				else if (key == out->data.first) return iterator(out);
				else out = out->right;
			}
			return end();
		}
		inline const_iterator find(const KEY& key) const { return const_iterator(const_cast<tree<KEY, DATA>* >(this)->find(key)); } /**< See above. */

		/** Inserts a new data element with its key.
			If the key already exists within the tree, the
			old data element will be overwritten using the new data element.
			@param key The new key.
			@param data The new data element.
			@return An iterator that points to the tree element. */
		iterator insert(const KEY& key, const DATA& data)
		{
			// First step: look for an already existing entry.
			node** insertAt = &root->right,* parent = root;
			while (*insertAt != NULL)
			{
				parent = *insertAt;
				if (key < parent->data.first) insertAt = &parent->left;
				else if (key == parent->data.first)
				{
					parent->data.second = data;
					return iterator(parent);
				}
				else insertAt = &parent->right;
			}

			// Insert the new node.
			node* n = ((*insertAt) = (node*) fm::Allocate(sizeof(node)));
			fm::Construct(*insertAt); // could be get rid of this one? it would work for all pointer and primitive types..
			n->parent = parent;
			n->data.first = key;
			n->data.second = data;
			++sized;
			
			// Balance the tree.
			parent->weight += (*insertAt == parent->right) ? 1 : -1;
			node* it = parent;
			while (it != root)
			{
				// Check whether we need to balance this level.
				if (it->weight > 1)
				{
					if (it->right->weight < 0) it->right->rotateRight();
					it->rotateLeft();
					it = it->parent;
					break;
				}
				else if (it->weight < -1)
				{
					if (it->left->weight > 0) it->left->rotateLeft();
					it->rotateRight();
					it = it->parent;
					break;
				}
				else if (it->weight == 0) break; // no height change.

				// go up one level.
				it->parent->weight += (it == it->parent->right) ? 1 : -1;
				it = it->parent;
			}

#ifdef TREE_DEBUG
			root->right->is_correct();
#endif // TREE_DEBUG

			return iterator(n);
		}
		
		/** Retrieves a data element using its key.
			@param k The key.
			@return The data element for this key. In the non-constant
				version of this function, a new element is created for
				the key if it does not already belong to the tree. */
		inline DATA& operator[](const KEY& k) { iterator it = find(k); if (it != end()) return it->second; else { DATA d; return insert(k, d)->second; } }
		inline const DATA& operator[](const KEY& k) const { return find(k)->second; } /**< See above. */

		/** Removes a data element from the tree.
			@param key The key of the data element to erase. */
		inline void erase(const KEY& key) { iterator it = find(key); erase(it); }
		
		/** Removes a data element from the tree.
			@param it An iterator that points to the tree element to erase. */
		inline void erase(const iterator& it)
		{
			node* n = it.currentNode;
			if (n != root)
			{
				node* release;
				if (n->left == NULL && n->right == NULL) release = n;
				else
				{
					// choose whether to reduce on the left or right.
					if (n->weight <= 0 && n->left != NULL)
					{
						// take out the left's rightmost node.
						release = n->left;
						while (release->right != NULL) release = release->right;
						n->data = release->data;

						// push up any left node on the rightmost node.
						if (release->left != NULL)
						{
							release->data = release->left->data;
							release = release->left;
						}
					}
					else
					{
						// take out the right's leftmost node.
						release = n->right;
						while (release->left != NULL) release = release->left;
						n->data = release->data;

						// push up any right node on the leftmost node.
						if (release->right != NULL)
						{
							release->data = release->right->data;
							release = release->right;
						}
					}
				}

				// Release the selected node and re-adjust its parent's weight.
				node* rebalance = release->parent;
				if (rebalance->left == release) { rebalance->left = NULL; ++rebalance->weight; }
				else { rebalance->right = NULL; --rebalance->weight; }
				release->data.first.~KEY();
				release->data.second.~DATA();
				fm::Release(release);
				--sized;

				// Rebalance the tree.
				node* it = rebalance;
				while (it != root)
				{
					// Check whether we need to balance this level.
					if (it->weight > 1)
					{
						if (it->right->weight < 0) it->right->rotateRight();
						it->rotateLeft();
						it = it->parent;
					}
					else if (it->weight < -1)
					{
						if (it->left->weight > 0) it->left->rotateLeft();
						it->rotateRight();
						it = it->parent;
					}
					if (it->weight != 0) break;

					// go up one level.
					it->parent->weight -= (it == it->parent->right) ? 1 : -1;
					it = it->parent;
				}
			}

#ifdef TREE_DEBUG
			if (root->right != NULL) root->right->is_correct();
#endif // TREE_DEBUG
		}

		/** Retrieves whether the tree contains any data nodes.
			@return Whether the tree contains any data nodes. */
		inline bool empty() const { return sized == 0; }

		/** Retrieves the number of data nodes contained in the tree.
			@return The number of data nodes contained in the tree. */
		inline size_t size() const { return sized; }

		/** Removes all the data nodes from the tree.
			This effectively prunes at the tree root. */
		void clear()
		{
			// Need to delete all the nodes.
			if (root->right != NULL)
			{
				node* n = root->right;
				while (n != root)
				{
					if (n->left != NULL) n = n->left;
					else if (n->right != NULL) n = n->right;
					else
					{
						// destroy this node.
						node* release = n;
						n = n->parent;
						if (n->left == release) n->left = NULL;
						else if (n->right == release) n->right = NULL;
						release->data.first.~KEY();
						release->data.second.~DATA();
						fm::Release(release);
						--sized;
					}
				}
				root->right = NULL;
			}
		}

		/** Copy constructor. Clones another tree into this one.
			This is a bad function: it costs too much performance, avoid at all costs!
			@param copy The tree to clone.
			@return This tree, cloned. */			
		inline tree<KEY,DATA>& operator= (const tree<KEY,DATA>& copy)
		{
			clear();

			// Function based on the iterator..
			// Go one right or one up.
			node* currentNode = copy.root;
			node* cloneNode = root;
			if (currentNode->right != NULL)
			{
				do
				{
					if (currentNode->right == NULL)
					{
						const node* oldNode;
						do
						{
							oldNode = currentNode;

							// Go one up.
							// We control the root node, which is the only with parent == NULL.
							// if you crash here, you don't check for iterator == end() correctly.
							currentNode = currentNode->parent;
							cloneNode = cloneNode->parent;
						}
						while (currentNode->right == oldNode && currentNode->parent != NULL);
					}
					else
					{
						// Create and go one right.
						currentNode = currentNode->right;
						
						cloneNode->right = (node*) fm::Allocate(sizeof(node));
						fm::Construct(cloneNode->right);
						cloneNode->right->parent = cloneNode;
						cloneNode->right->data = currentNode->data;
						cloneNode->right->weight = currentNode->weight;
						++sized;

						cloneNode = cloneNode->right;

						// Create and go one all the way left.
						while (currentNode->left != NULL)
						{
							currentNode = currentNode->left;

							cloneNode->left = (node*) fm::Allocate(sizeof(node));
							fm::Construct(cloneNode->left);
							cloneNode->left->parent = cloneNode;
							cloneNode->left->data = currentNode->data;
							cloneNode->left->weight = currentNode->weight;
							++sized;

							cloneNode = cloneNode->left;
						}
					}
				}
				while (currentNode != copy.root);
			}

			return (*this);
		}
	};

	template <class KEY, class DATA>
	bool tree<KEY,DATA>::iterator::operator==(const const_iterator& other) const { return other.currentNode == currentNode; }
	template <class KEY, class DATA>
	bool tree<KEY,DATA>::iterator::operator!=(const const_iterator& other) const { return other.currentNode != currentNode; }

	/** A STL set. */
	template <class _Kty>
	class set : public fm::tree<_Kty, _Kty> {};

	/** A STL map. */
	template <class _Kty, class _Ty>
	class map : public fm::tree<_Kty, _Ty> {};
};

#endif // _FM_TREE_H_


