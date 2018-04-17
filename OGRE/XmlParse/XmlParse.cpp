#include <stdio.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "rapidxml.hpp"
using namespace std;
using namespace rapidxml;

typedef unsigned int uint;

class InstructionStorage
{
	struct context
	{
		uint addr;
		string command;
		string arg1;
		string arg2;
	};

	uint orgAddr, espAddr, stackSize;
	vector<context> vStorage;

public:

	void parseXML(string filename)
	{
		xml_document<> doc;
		ifstream file(filename);

		if (!file.is_open())
			throw "File not found";

		stringstream buffer;
		buffer << file.rdbuf();
		file.close();

		string content(buffer.str());
		doc.parse<0>(&content[0]);

		xml_node<> *pRoot = doc.first_node();

		if (pRoot == NULL)
			throw "Error while parsing xml: pRoot was null";

		////////////// first get setup attributes

		xml_node<> *pNode;
		xml_attribute<> *pAttr;
		string strVal;

		pNode = pRoot->first_node("setup");
		pAttr = pNode->first_attribute("org_address");
		orgAddr = stoi(pAttr->value(), NULL, 16);

		pNode = pNode->next_sibling();
		pAttr = pNode->first_attribute("esp_address");
		espAddr = stoi(pAttr->value(), NULL, 16);

		pNode = pNode->next_sibling();
		pAttr = pNode->first_attribute("stack_size");
		stackSize = stoi(pAttr->value(), NULL, 16);

		if (orgAddr > 0xFFFFF000)
			throw "Invalid parameter org_address";
		if (stackSize > espAddr)
			throw "Invalid parameters esp_address, stack_size";

		///////////// then load all instructions with arguments to vector

		int i = 0;
		for (xml_node<> *pNode = pRoot->first_node("command"); pNode; pNode = pNode->next_sibling())
		{
			context con;
			con.addr = i + orgAddr;
			con.command = pNode->first_attribute("name")->value();


			xml_node<> *pInnerNode = pNode->first_node("argument");

			if (pInnerNode == NULL)
			{
				con.arg1 = "";
				con.arg2 = "";
				goto end_loop;
			}

			else
				con.arg1 = pInnerNode->first_attribute("value")->value();
			
			pInnerNode = pInnerNode->next_sibling();

			if (pInnerNode == NULL)
				con.arg2 = "";
			else
				con.arg2 = pInnerNode->first_attribute("value")->value();

		end_loop:
			
			vStorage.push_back(con);
			i++;
		}
	}

	void print()
	{
		printf("org = %x, esp = %x, stack = %x\n", orgAddr, espAddr, stackSize);

		for (auto& e : vStorage)
		{
			printf("%08X %s %s %s\n", e.addr, e.command.c_str(), e.arg1.c_str(), e.arg2.c_str());
		}
	}

	string getCurInstructionText(int i)
	{
		char buf[50] = { 0 };
		context con = vStorage
		sprintf(buf, "%08X %s %s %s\n", e.addr, e.command.c_str(), e.arg1.c_str(), e.arg2.c_str());
	}
};


int main()
{
	try
	{
		InstructionStorage s;
		s.parseXML("xml_file.xml");
		s.print();
	}

	catch (char *x)
	{
		printf("Exception occured: %s", x);
	}

	catch (exception e)
	{
		printf("Parse xml error occured: %s", e.what());
	}

	catch (...)
	{
		printf("Unknown exception occured\n");
	}

}