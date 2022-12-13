#include <stdio.h>
#include <stdlib.h>

#define GVDLL
#include <graphviz/gvc.h>
#include <graphviz/gvplugin.h>


#include <iostream>
#include <queue>
using namespace std;

enum COLOR { RED, BLACK };

class Node {
public:
	int val;
	COLOR color;
	Node* left, * right, * parent;

	Node(int val) : val(val) {
		parent = left = right = NULL;

		// Node is created during insertion
		// Node is red at insertion
		color = RED;
	}

	// returns pointer to uncle
	Node* uncle() {
		// If no parent or grandparent, then no uncle
		if (parent == NULL || parent->parent == NULL)
			return NULL;

		if (parent->isOnLeft())
			// uncle on right
			return parent->parent->right;
		else
			// uncle on left
			return parent->parent->left;
	}

	// check if node is left child of parent
	bool isOnLeft() { return this == parent->left; }

	// returns pointer to sibling
	Node* sibling() {
		// sibling null if no parent
		if (parent == NULL)
			return NULL;

		if (isOnLeft())
			return parent->right;

		return parent->left;
	}

	// moves node down and moves given node in its place
	void moveDown(Node* nParent) {
		if (parent != NULL) {
			if (isOnLeft()) {
				parent->left = nParent;
			}
			else {
				parent->right = nParent;
			}
		}
		nParent->parent = parent;
		parent = nParent;
	}

	bool hasRedChild() {
		return (left != NULL && left->color == RED) ||
			(right != NULL && right->color == RED);
	}
};

class RBTree {
	Node* root;

	// left rotates the given node
	void leftRotate(Node* x) {
		// new parent will be node's right child
		Node* nParent = x->right;

		// update root if current node is root
		if (x == root)
			root = nParent;

		x->moveDown(nParent);

		// connect x with new parent's left element
		x->right = nParent->left;
		// connect new parent's left element with node
		// if it is not null
		if (nParent->left != NULL)
			nParent->left->parent = x;

		// connect new parent with x
		nParent->left = x;
	}

	void rightRotate(Node* x) {
		// new parent will be node's left child
		Node* nParent = x->left;

		// update root if current node is root
		if (x == root)
			root = nParent;

		x->moveDown(nParent);

		// connect x with new parent's right element
		x->left = nParent->right;
		// connect new parent's right element with node
		// if it is not null
		if (nParent->right != NULL)
			nParent->right->parent = x;

		// connect new parent with x
		nParent->right = x;
	}

	void swapColors(Node* x1, Node* x2) {
		COLOR temp;
		temp = x1->color;
		x1->color = x2->color;
		x2->color = temp;
	}

	void swapValues(Node* u, Node* v) {
		int temp;
		temp = u->val;
		u->val = v->val;
		v->val = temp;
	}

	// fix red red at given node
	void fixRedRed(Node* x) {
		// if x is root color it black and return
		if (x == root) {
			x->color = BLACK;
			return;
		}

		// initialize parent, grandparent, uncle
		Node* parent = x->parent, * grandparent = parent->parent,
			* uncle = x->uncle();

		if (parent->color != BLACK) {
			if (uncle != NULL && uncle->color == RED) {
				// uncle red, perform recoloring and recurse
				parent->color = BLACK;
				uncle->color = BLACK;
				grandparent->color = RED;
				fixRedRed(grandparent);
			}
			else {
				// Else perform LR, LL, RL, RR
				if (parent->isOnLeft()) {
					if (x->isOnLeft()) {
						// for left right
						swapColors(parent, grandparent);
					}
					else {
						leftRotate(parent);
						swapColors(x, grandparent);
					}
					// for left left and left right
					rightRotate(grandparent);
				}
				else {
					if (x->isOnLeft()) {
						// for right left
						rightRotate(parent);
						swapColors(x, grandparent);
					}
					else {
						swapColors(parent, grandparent);
					}

					// for right right and right left
					leftRotate(grandparent);
				}
			}
		}
	}

	// find node that do not have a left child
	// in the subtree of the given node
	Node* successor(Node* x) {
		Node* temp = x;

		while (temp->left != NULL)
			temp = temp->left;

		return temp;
	}

	// find node that replaces a deleted node in BST
	Node* BSTreplace(Node* x) {
		// when node have 2 children
		if (x->left != NULL && x->right != NULL)
			return successor(x->right);

		// when leaf
		if (x->left == NULL && x->right == NULL)
			return NULL;

		// when single child
		if (x->left != NULL)
			return x->left;
		else
			return x->right;
	}

	// deletes the given node
	void deleteNode(Node* v) {
		Node* u = BSTreplace(v);

		// True when u and v are both black
		bool uvBlack = ((u == NULL || u->color == BLACK) && (v->color == BLACK));
		Node* parent = v->parent;

		if (u == NULL) {
			// u is NULL therefore v is leaf
			if (v == root) {
				// v is root, making root null
				root = NULL;
			}
			else {
				if (uvBlack) {
					// u and v both black
					// v is leaf, fix double black at v
					fixDoubleBlack(v);
				}
				else {
					// u or v is red
					if (v->sibling() != NULL)
						// sibling is not null, make it red"
						v->sibling()->color = RED;
				}

				// delete v from the tree
				if (v->isOnLeft()) {
					parent->left = NULL;
				}
				else {
					parent->right = NULL;
				}
			}
			delete v;
			return;
		}

		if (v->left == NULL || v->right == NULL) {
			// v has 1 child
			if (v == root) {
				// v is root, assign the value of u to v, and delete u
				v->val = u->val;
				v->left = v->right = NULL;
				delete u;
			}
			else {
				// Detach v from tree and move u up
				if (v->isOnLeft()) {
					parent->left = u;
				}
				else {
					parent->right = u;
				}
				delete v;
				u->parent = parent;
				if (uvBlack) {
					// u and v both black, fix double black at u
					fixDoubleBlack(u);
				}
				else {
					// u or v red, color u black
					u->color = BLACK;
				}
			}
			return;
		}

		// v has 2 children, swap values with successor and recurse
		swapValues(u, v);
		deleteNode(u);
	}

	void fixDoubleBlack(Node* x) {
		if (x == root)
			// Reached root
			return;

		Node* sibling = x->sibling(), * parent = x->parent;
		if (sibling == NULL) {
			// No sibiling, double black pushed up
			fixDoubleBlack(parent);
		}
		else {
			if (sibling->color == RED) {
				// Sibling red
				parent->color = RED;
				sibling->color = BLACK;
				if (sibling->isOnLeft()) {
					// left case
					rightRotate(parent);
				}
				else {
					// right case
					leftRotate(parent);
				}
				fixDoubleBlack(x);
			}
			else {
				// Sibling black
				if (sibling->hasRedChild()) {
					// at least 1 red children
					if (sibling->left != NULL && sibling->left->color == RED) {
						if (sibling->isOnLeft()) {
							// left left
							sibling->left->color = sibling->color;
							sibling->color = parent->color;
							rightRotate(parent);
						}
						else {
							// right left
							sibling->left->color = parent->color;
							rightRotate(sibling);
							leftRotate(parent);
						}
					}
					else {
						if (sibling->isOnLeft()) {
							// left right
							sibling->right->color = parent->color;
							leftRotate(sibling);
							rightRotate(parent);
						}
						else {
							// right right
							sibling->right->color = sibling->color;
							sibling->color = parent->color;
							leftRotate(parent);
						}
					}
					parent->color = BLACK;
				}
				else {
					// 2 black children
					sibling->color = RED;
					if (parent->color == BLACK)
						fixDoubleBlack(parent);
					else
						parent->color = BLACK;
				}
			}
		}
	}

	// prints level order for given node
	void levelOrder(Node* x) {
		if (x == NULL)
			// return if node is null
			return;

		// queue for level order
		queue<Node*> q;
		Node* curr;

		// push x
		q.push(x);

		while (!q.empty()) {
			// while q is not empty
			// dequeue
			curr = q.front();
			q.pop();

			// print node value
			cout << curr->val << " ";

			// push children to queue
			if (curr->left != NULL)
				q.push(curr->left);
			if (curr->right != NULL)
				q.push(curr->right);
		}
	}

	// prints inorder recursively
	void inorder(Node* x) {
		if (x == NULL)
			return;
		inorder(x->left);
		cout << x->val << " ";
		inorder(x->right);
	}

public:
	// constructor
	// initialize root
	RBTree() { root = NULL; }

	Node* getRoot() { return root; }

	// searches for given value
	// if found returns the node (used for delete)
	// else returns the last node while traversing (used in insert)
	Node* search(int n) {
		Node* temp = root;
		while (temp != NULL) {
			if (n < temp->val) {
				if (temp->left == NULL)
					break;
				else
					temp = temp->left;
			}
			else if (n == temp->val) {
				break;
			}
			else {
				if (temp->right == NULL)
					break;
				else
					temp = temp->right;
			}
		}

		return temp;
	}

	// inserts the given value to tree
	void insert(int n) {
		Node* newNode = new Node(n);
		if (root == NULL) {
			// when root is null
			// simply insert value at root
			newNode->color = BLACK;
			root = newNode;
		}
		else {
			Node* temp = search(n);

			if (temp->val == n) {
				// return if value already exists
				return;
			}

			// if value is not found, search returns the node
			// where the value is to be inserted

			// connect new node to correct node
			newNode->parent = temp;

			if (n < temp->val)
				temp->left = newNode;
			else
				temp->right = newNode;

			// fix red red voilaton if exists
			fixRedRed(newNode);
		}
	}

	// utility function that deletes the node with given value
	void deleteByVal(int n) {
		if (root == NULL)
			// Tree is empty
			return;

		Node* v = search(n), * u;

		if (v->val != n) {
			cout << "No node found to delete with value:" << n << endl;
			return;
		}

		deleteNode(v);
	}

	// prints inorder of the tree
	void printInOrder() {
		cout << "Inorder: " << endl;
		if (root == NULL)
			cout << "Tree is empty" << endl;
		else
			inorder(root);
		cout << endl;
	}

	// prints level order of the tree
	void printLevelOrder() {
		cout << "Level order: " << endl;
		if (root == NULL)
			cout << "Tree is empty" << endl;
		else
			levelOrder(root);
		cout << endl;
	}
};


#include<graphviz/gvc.h>

//#define SINGLE_NIL   // 将所有的NIL当作一个结点

char __node_name[20];

char* get_node_name(int val, char* leaf_str)
{
	if (leaf_str == NULL)
	{
		_itoa_s(val, __node_name, 10);
		return __node_name;
	}
	else {
#ifdef SINGLE_NIL
		// 如果结点的name全部都为空字符串，那么实际上agnode获取到的都是同一个结点
		// 从而实现了将所有的叶结点当作同一个结点的需求
		return "";
	}
#else
		// 从这里返回的每个叶结点的name都不相同，因此agnode获取到的都是不同的叶结点
		// 从而实现了每个叶结点单独显示的需求
		sprintf_s(__node_name, "%s/%d", leaf_str, val);
		return __node_name;
}
#endif
}

Agnode_t* __draw_rb_node(Agraph_t* g, Agnode_t* _parent, char* name, char* label, int color)
{
	Agnode_t* _node = agnode(g, name, 1);
	if (label != NULL)
	{
		// 如果注释下面这一句, 不设置label, 输出的图上结点显示name属性
		// 设置了label之后则会显示label
		agsafeset(_node, "label", label, "");
	}
	// 必须加这一句，否则下面的fillcolor属性设置无效
	agsafeset(_node, "style", "filled", "");
	if (color == 0)
	{
		agsafeset(_node, "fillcolor", "red", "");   // 填充红色
		agsafeset(_node, "fontcolor", "white", ""); // 字体白色
	}
	else {
		agsafeset(_node, "fillcolor", "black", ""); // 填充黑色
		agsafeset(_node, "fontcolor", "white", ""); // 字体白色
	}
	if (_parent != NULL)
	{
		agedge(g, _parent, _node, NULL, 1);
	}
#ifdef SINGLE_NIL
	// 启用SINGLE_NIL, 绘制类似于算法导论175页中的图13-1(b)
	else {
		// 如果parent为NULL，说明该结点是根结点，因此获取NIL结点作为根结点的父节点
		// agnode的第二个参数name如果和已经存在的某个结点一样，那么agnode直接返回已存在的结点
		// 将name置为空，获取全图唯一的NIL结点
		Agnode_t* root_parent = agnode(g, "", 1);
		agsafeset(root_parent, "label", "NIL", "");
		agsafeset(root_parent, "style", "filled", "");
		agsafeset(root_parent, "fillcolor", "black", "");
		agsafeset(root_parent, "fontcolor", "white", "");
		agedge(g, _node, root_parent, NULL, 1);
	}
#endif
	return _node;
}

void __draw_rb(Agraph_t* g, Agnode_t* _parent, Node* parent_node, Node* _node)
{
	if (_node != NULL)
	{
		// 当前结点不是叶结点
		_parent = __draw_rb_node(g, _parent, get_node_name(_node->val, NULL), NULL, _node->color);
	}

	if (_node->left != NULL)
	{
		// 左孩子不是叶结点
		__draw_rb(g, _parent, _node, _node->left);
	}
	else {
		// 左孩子是叶结点 NIL
		__draw_rb_node(g, _parent, get_node_name(_node->val, "lnil"), "NIL", 1);
	}

	if (_node->right != NULL)
	{
		// 右孩子不是叶结点
		__draw_rb(g, _parent, _node, _node->right);
	}
	else {
		// 右孩子是叶结点 NIL
		__draw_rb_node(g, _parent, get_node_name(_node->val, "rnil"), "NIL", 1);
	}
	return;
}





void draw_rb(Node* root)
{
	// 1 创建上下文，存储graph和渲染方法等
	GVC_t* gvc = gvContext();

	// 2 创建一个 graph
	Agraph_t* g = agopen("g", Agdirected, 0);
	// 3 开始绘图
	__draw_rb(g, NULL, NULL, root);  // rbtree绘图方法
	// 4 指定输出的图像质量
	//agsafeset(g, "dpi", "600", "");
	// 5 指定布局方式，文档中有8种布局方式可选，这里选择"dot"生成红黑树
	gvLayout(gvc, g, "dot");
	// 6 输出图片
	//gvRenderFilename(gvc, g, "png", "test.png");
	gvRenderFilename(gvc, g, "pdf", "test.pdf");
	//gvRenderFilename(gvc, g, "svg", "test.svg");

	// 7 释放内存
	gvFreeLayout(gvc, g);
	agclose(g);
	gvFreeContext(gvc);
}


int rb_main() {
	RBTree tree;

	tree.insert(20);
	tree.insert(12);
	tree.insert(23);
	tree.insert(11);
	tree.insert(15);
	tree.insert(22);
	tree.insert(32);
	tree.insert(5);
	tree.insert(13);
	tree.insert(17);
	tree.insert(25);
	tree.insert(33);

	draw_rb(tree.getRoot());
	return 0;
}

int main(void) {
	rb_main();
	return EXIT_SUCCESS;
}
