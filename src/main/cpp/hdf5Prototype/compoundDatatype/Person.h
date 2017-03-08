#include <string>

using namespace std;

struct PersonData {
	int age;
	int height;
	char gender;
};


class Person {
public:
	Person();
	Person(int age, int height, char name);
	inline int getAge() const { return m_data.age; }
	inline int getHeight() const { return m_data.height; }
	inline char getGender() const { return m_data.gender; }

private:
	PersonData m_data;
};


