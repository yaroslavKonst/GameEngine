#ifndef _CODE_TREE
#define _CODE_TREE

#include <map>

#include "BitHelper.h"

class CodeTree
{
public:
	CodeTree()
	{
	}

	~CodeTree()
	{
		for (auto& node : _freeNodes) {
			DeleteNode(node.second);
		}
	}

	void AddSymbol(uint8_t symbol, size_t weight)
	{
		Node* node = new Node;
		node->Symbols.insert(symbol);
		node->Weight = weight;
		node->Left = nullptr;
		node->Right = nullptr;

		_freeNodes.insert({weight, node});
	}

	void Build()
	{
		while (_freeNodes.size() > 1) {
			auto minNodeIt = _freeNodes.begin();
			Node* minNode1 = minNodeIt->second;
			_freeNodes.erase(minNodeIt);

			minNodeIt = _freeNodes.begin();
			Node* minNode2 = minNodeIt->second;
			_freeNodes.erase(minNodeIt);

			Node* mergeNode = new Node;
			mergeNode->Weight =
				minNode1->Weight + minNode2->Weight;
			mergeNode->Left = minNode1;
			mergeNode->Right = minNode2;

			for (uint8_t symbol : minNode1->Symbols) {
				mergeNode->Symbols.insert(symbol);
			}

			for (uint8_t symbol : minNode2->Symbols) {
				mergeNode->Symbols.insert(symbol);
			}

			_freeNodes.insert({mergeNode->Weight, mergeNode});
		}
	}

	std::vector<uint8_t> GetCode(uint8_t symbol, size_t& codeSize)
	{
		BitWriter writer;

		Node* node = _freeNodes.begin()->second;

		if (node->Symbols.size() <= 1) {
			writer.Put(0);
		}

		while (node->Symbols.size() > 1) {
			if (
				node->Left->Symbols.find(symbol) !=
				node->Left->Symbols.end())
			{
				node = node->Left;
				writer.Put(0);
			} else {
				node = node->Right;
				writer.Put(1);
			}
		}

		return writer.Get(codeSize);
	}

private:
	struct Node
	{
		std::set<uint8_t> Symbols;
		size_t Weight;

		// Bit 0
		Node* Left;

		// Bit 1
		Node* Right;
	};

	std::multimap<size_t, Node*> _freeNodes;

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
