#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <omp.h>
using namespace std;

int randomNumber(mt19937& randomGenerator)
{
	uniform_int_distribution<int> distribution(0, 100);
	return distribution(randomGenerator);
}

class BinTree // Класс "Бинарное дерево"
{
private:
    struct Node // Структура "Узел дерева"
    {
        int data; // Поле данных
        Node* left = nullptr; // Левый узел
        Node* right = nullptr; // Правый узел
        Node* parent = nullptr; // Родительский узел
        short depth = 0; // Глубина узла
        int subTreeNodeCount = 0; // Количество узлов в поддереве
    };
    void Add(const int& data, Node* current, Node* parent, short depth); // Рекурсивный метод добавления
    void RemoveSubtree(Node* subroot); // Удаление поддерева
    int maxElement = INT_MIN; // Максимальный элемент дерева
    short maxDepth = 0; // Максимальная глубина дерева
    vector<bool> formatControl{false}; // Корректировка для форматирования вывода дерева

public:
    Node* root; // Корень дерева
    BinTree() : root(nullptr) {}; // Конструктор
    ~BinTree(); // Деструктор
    void AddNode(const int& data); // Добавление нового узла
    void Print(Node* node); // Вывод узлов дерева на экран
    int FindMaxNode(Node* root); // Поиск максимального элемента дерева
};

BinTree::~BinTree()
{
    RemoveSubtree(root);
}

void BinTree::RemoveSubtree(Node* subroot)
{
    if (subroot != nullptr)
    {
        RemoveSubtree(subroot->left); // Удаляем левую часть дерева.
        RemoveSubtree(subroot->right); // Удаляем правую часть дерева.

        delete subroot;
    }
}

void BinTree::AddNode(const int& data)
{
    if (root == nullptr)
    {
        root = new Node; // Выделяем пямять под корень дерева
        root->data = data; // Заполняем корень данными
        root->depth++;
        root->subTreeNodeCount++;
        maxDepth = root->depth;
        formatControl.emplace_back(false);
    }
    else
    {
        if (root->subTreeNodeCount % 2) // Условие добавления левого потомка
        {
            Add(data, root->left, root, root->depth + 1);
            root->subTreeNodeCount++;
        }
        else // Условие добавления правого потомка
        {
            Add(data, root->right, root, root->depth + 1);
            root->subTreeNodeCount++;
        }
    }
}

void BinTree::Add(const int& data, Node* current, Node* parent, short depth)
{
    if (current == nullptr)
    {
        Node* node = new Node; // Выделяем память под узел
        node->data = data; // Заполняем узел данными
        node->parent = parent; // Связываем узел с родительским
        node->depth = depth;
        node->subTreeNodeCount++;
        if (maxDepth < depth)
        {
            maxDepth = depth;
            formatControl.emplace_back(false);
        }

        if (parent->subTreeNodeCount % 2)
        {
            parent->left = node;
        }
        else
        {
            parent->right = node;
        }
    }
    else
    {
        if (current->subTreeNodeCount % 2) // Условие добавления левого потомка
        {
            Add(data, current->left, current, ++depth);
            current->subTreeNodeCount++;
        }
        else // Условие добавления правого потомка
        {
            Add(data, current->right, current, ++depth);
            current->subTreeNodeCount++;
        }
    }
}

void BinTree::Print(Node* node)
{
    if(node == nullptr)
    {
        return;
    }
    Print(node->right);
    for(int i = 1; i < node->depth; i++)
    {
        if (i + 1 == node->depth and node == node->parent->right)
        {
            cout << " ┌── ";
            formatControl.at(node->depth - 1) = true;
        }
        else if (i + 1 == node->depth and node == node->parent->left)
        {
            cout << " └── ";
            formatControl.at(node->depth - 1) = false;
        }
        else if (formatControl.at(i))
        {
            cout << " │  ";
        }
        else
        {
            cout << "    ";
        }
    }
    cout << (node == root? " " : "") << node->data << endl;
    if (node->left != nullptr)
    {
        formatControl.at(node->depth) = true;
    }
    else
    {
        formatControl.at(node->depth) = false;
    }
    Print(node->left);
}

int BinTree::FindMaxNode(Node* node)
{
    if (node != nullptr)
    {
        #pragma omp task
        {
            FindMaxNode(node->left);
        }
        #pragma omp critical
        {
            if (maxElement < node->data)
            {
                maxElement = node->data;
            }
        }
        #pragma omp task
        {
            FindMaxNode(node->right);
        }
    }
    return maxElement;
}

int main()
{
    setlocale(LC_ALL, "ru_RU.UTF8");
    mt19937 randomGenerator;
	random_device device;
	randomGenerator.seed(device());

    unsigned treeSize;
	cout << "Введите количество узлов бинарного дерева: ";
	cin >> treeSize;
    
    BinTree tree;
    cout << "Заполнение бинарного дерева..." << endl;
    for (unsigned i = 0; i < treeSize; i++)
    {
        tree.AddNode(randomNumber(randomGenerator));
    }

    if (treeSize <= 127)
    {
        tree.Print(tree.root);
    }

    int maxNode;
    double start, finish, elapsedTime;
    cout << "Поиск максимального элемента..." << endl << endl;

    cout << "#================================#" << endl;
    cout << "Последовательный метод: " << endl;
    start = omp_get_wtime();
    maxNode = tree.FindMaxNode(tree.root);
    finish = omp_get_wtime();
    elapsedTime = finish - start;
    cout << "Максимальный элемент дерева: " << maxNode << endl;
    cout << "Затраченное время: " << setprecision(3) << elapsedTime
        <<" с. (" << elapsedTime * 1000.0 << " мс.)" << endl << endl;

    cout << "----------------------------------" << endl << endl;

    cout << "Параллельный метод: " << endl;
    start = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
            maxNode = tree.FindMaxNode(tree.root);
        }
    }
    finish = omp_get_wtime();
    elapsedTime = finish - start;
    cout << "Максимальный элемент дерева: " << maxNode << endl;
    cout << "Затраченное время: " << setprecision(3) << elapsedTime
        << " с. (" << elapsedTime * 1000.0 << " мс.)" << endl;
    cout << "#================================#" << endl << endl;

    cout << "Освобождение памяти..." << endl;
    return 0;
}