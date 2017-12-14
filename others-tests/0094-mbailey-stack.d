// Error: undeclared variable: Top
int x;

class Stack {
      void init() {
      	   Top = NULL;
      }
      void push(int value) {
       	   Node node;

	   node = New(Node);
	   node.init(value, Top);
	   Top = node;
      }
      int top() {
      	   return Top.Value;
      }
      int pop() {
      	   Top = Top.Next;
      }
      bool empty() {
      	   return Top == NULL;
      }
      
      Node Top;
}

class Node {
      int Value;
      Node Next;
      void init(int value, Node next) {
      	   Value = value;
	   Next = next;	   
      }
}

int main() {
    Stack s;

    s = New(Stack);
    s.init();
    s.push(10);
}
