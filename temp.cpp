#include <iostream>
using namespace std;

class test{
    public:
        test(int a):c(a){
            cout<<"init"<<endl;
        }
        ~test(){
            cout<<"dtor"<<endl;
        }
        test(test &a){
            cout<<"copy"<<endl;
            this->c = 5;
        }
        test(test &&a){
            cout<<"move"<<endl;
            this->c = a.c;
        }
        void print(){
            cout<<this->c<<endl;
        }
        test& operator=(test a){
            cout<<"=="<<endl;
            this->c = 5;
            return *this;
        }
    private:
        int c;
};

test gettest(){
    test a(1);
    return a;
}
int main(){
    // test b(2);
    // gettest();
    test c(gettest());
    c.print();
}