#include <boost/atomic.hpp>
#include <iostream>

int main(){
	bool volatile b = false;
	std::cout<<"aaa"<<std::endl;
	boost::atomic<double> out(0);
	out.store(1.1, boost::memory_order_release);
	std::cout<<out<<std::endl;
}