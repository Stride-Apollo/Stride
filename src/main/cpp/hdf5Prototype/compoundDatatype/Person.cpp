#include "Person.h"
#include <string>

using namespace std;

Person::Person() {
	m_data.age = 20;
	m_data.height = 175;
	m_data.gender = 'M';
}

Person::Person(int age, int height, char gender) {
	m_data.age = age;
	m_data.height = height;
	m_data.gender = gender;
}
