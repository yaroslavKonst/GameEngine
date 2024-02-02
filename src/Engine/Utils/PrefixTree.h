#ifndef _PREFIX_TREE
#define _PREFIX_TREE

#include "BitHelper.h"

class PrefixTree
{
public:
	PrefixTree()
	{
		_root = nullptr;
		_currentNode = nullptr;
	}

	~PrefixTree()
	{
		DeleteNode(_root);
	}

	void InsertValue(uint8_t symbol, BitReader* bitReader)
	{
		Node** currentNode = &_root;

		uint8_t bit;
		while (bitReader->Get(bit)) {
			if (!*currentNode) {
				*currentNode = new Node;
				(*currentNode)->Left = nullptr;
				(*currentNode)->Right = nullptr;
			}

			if (bit) {
				currentNode = &(*currentNode)->Right;
			} else {
				currentNode = &(*currentNode)->Left;
			}
		}

		if (!*currentNode) {
			*currentNode = new Node;
			(*currentNode)->Left = nullptr;
			(*currentNode)->Right = nullptr;
		}

		(*currentNode)->Symbol = symbol;
	}

	void Init()
	{
		_currentNode = _root;
	}

	bool NextBit(uint8_t bit, uint8_t& symbol)
	{
		if (bit) {
			_currentNode = _currentNode->Right;
		} else {
			_currentNode = _currentNode->Left;
		}

		if (!(_currentNode->Left || _currentNode->Right)) {
			symbol = _currentNode->Symbol;
			_currentNode = _root;
			return true;
		}

		return false;
	}

private:
	struct Node
	{
		uint8_t Symbol;

		// Bit 0
		Node* Left;

		// Bit 1
		Node* Right;
	};

	Node* _root;
	Node* _currentNode;

	void DeleteNode(Node* node)
	{
		if (!node) {
			return;
		}

		DeleteNode(node->Left);
		DeleteNode(node->Right);
		delete node;
	}
};

#endif
