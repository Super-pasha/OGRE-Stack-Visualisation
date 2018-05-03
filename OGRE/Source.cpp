#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreInput.h>
#include <OgreRTShaderSystem.h>
#include <OgreApplicationContext.h>
#include <OISInputManager.h>
#include <OISMouse.h>
#include <OISKeyboard.h>

#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>

#include "XmlParse\rapidxml.hpp"

using namespace Ogre;
using namespace OgreBites;
using namespace std;
using namespace rapidxml;


namespace Cubic
{
	const Ogre::Real scale_x = 2.5, scale_y = 2.5, scale_z = 2.5;	// k - how we increase cube
	const Ogre::Real size_x = 5, size_y = 5, size_z = 5;			// real length of cube edges
	const Ogre::Real delta_x = 2.5, delta_y = 2.5, delta_z = 2.5;	// distance between cubes

	const Ogre::Real cameraBoost = 3;
};

string uintValToHexString(uint val)
{
	char buf[9] = { 0 };
	sprintf(buf, "%08X", val);
	return buf;
}

class CubeLetter
/*
	Creates a letter as a cubic object with a picture
	Supports: a-z, space, 1-9, +-
*/
{
public:

	SceneNode* getSceneNode()
	{
		return node;
	}
		
	CubeLetter(char x, SceneNode* parent_Node, SceneManager* scnMgr, Vector3 loc)
	{
		val = x;

		if (x == ' ')
			mesh_val = "space.mesh";
		else if (x == '-')
			mesh_val = "minus.mesh";
		else
		{
			mesh_val = toupper(x);
			mesh_val += ".mesh";
		}

		mgr = scnMgr;
		ent = scnMgr->createEntity(mesh_val);
		node = parent_Node->createChildSceneNode();

		node->attachObject(ent);
		node->setScale(Cubic::scale_x, Cubic::scale_y, Cubic::scale_z);
		node->setPosition(loc);
	}

	~CubeLetter()
	{
		node->detachObject(ent);
		mgr->destroyEntity(ent);
		node->getParentSceneNode()->removeAndDestroyChild(node);
	}

private:

	char val;
	string rel_val;
	string mesh_val;

	Entity* ent;
	SceneNode* node;
	SceneManager* mgr;
};

class CubeText
{
	string strText;
	std::vector<CubeLetter*> cubeText;
	SceneNode* node;

public:

	SceneNode* getSceneNode()
	{
		return node;
	}

	string getStrText() 
	{
		return strText;
	}
	
	CubeText(string x, SceneNode* parent_Node, SceneManager* scnMgr, Vector3 loc = Vector3(0,0,0))
	{
		strText = x;
		node = parent_Node->createChildSceneNode();

		size_t n = x.length();

		for (size_t i = 0; i < n; i++)
		{
			Vector3 location = loc;
			location.x += i * Cubic::size_x;

			CubeLetter* cl = new CubeLetter(x[i], node, scnMgr, location);
			cubeText.push_back(cl);
		}
	}

	~CubeText()
	{
		while (!cubeText.empty())
		{
			CubeLetter* cl = cubeText.back();
			delete cl;
			cubeText.pop_back();
		}

		node->getParentSceneNode()->removeAndDestroyChild(node);
	}
};

class CubeRegister
{
	CubeText* name;
	string sname;
	SceneNode* node;

	CubeText* textVal;
	uint val;

	SceneManager* scnMgr;

	void clear_values()
	{
		if (textVal)
			delete textVal;

		val = 0;
	}

public:

	void set_value(uint x)
	{
		clear_values();

		Vector3 location = node->getPosition();
		textVal = new CubeText(uintValToHexString(x), node, scnMgr, location);
	
		val = x;
	}

	CubeRegister(string regName, SceneNode* parent_Node, SceneManager* _scnMgr, Vector3 loc)
	{
		node = parent_Node->createChildSceneNode();
		node->setPosition(loc);
		scnMgr = _scnMgr;
		sname = regName;

		Vector3 location = loc;
		location.x -= Cubic::size_x * 4;

		name = new CubeText(regName, node, scnMgr, location);
		val = 0;
		textVal = NULL;
	}

	~CubeRegister()
	{
		clear_values();
		delete name;
		node->getParentSceneNode()->removeAndDestroyChild(node);
	}

	CubeText* getText()
	{
		return textVal;
	}

	uint getValue()
	{
		return val;
	}

	string getName()
	{
		return sname;
	}

	SceneNode* getSceneNode()
	{
		return node;
	}
};

class CubeRegisterList
{
	std::map<string, CubeRegister*> *regMap;
	SceneNode* node;

	CubeRegister* eax;
	CubeRegister* ebx;
	CubeRegister* ecx;
	CubeRegister* edx;
	CubeRegister* eip;
	CubeRegister* esp;
	CubeRegister* ebp;

public:

	CubeRegisterList(SceneNode* parent_Node, SceneManager* _scnMgr, Vector3 loc)
	{
		node = parent_Node->createChildSceneNode();
		node->setPosition(loc);
		regMap = new std::map<string, CubeRegister*>();

		eax = new CubeRegister("eax", node, _scnMgr, Vector3(loc.x, loc.y, loc.z));
		ebx = new CubeRegister("ebx", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y, loc.z));
		ecx = new CubeRegister("ecx", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y * 2, loc.z));
		edx = new CubeRegister("edx", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y * 3, loc.z));
		eip = new CubeRegister("eip", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y * 4, loc.z));
		esp = new CubeRegister("esp", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y * 5, loc.z));
		ebp = new CubeRegister("ebp", node, _scnMgr, Vector3(loc.x, loc.y - Cubic::delta_y * 6, loc.z));

		eax->set_value(0);
		ebx->set_value(0);
		ecx->set_value(0);
		edx->set_value(0);
		esp->set_value(0);
		eip->set_value(0);
		ebp->set_value(0);

		regMap->insert(pair<string, CubeRegister*>("eax", eax));
		regMap->insert(pair<string, CubeRegister*>("ebx", ebx));
		regMap->insert(pair<string, CubeRegister*>("ecx", ecx));
		regMap->insert(pair<string, CubeRegister*>("edx", edx));
		regMap->insert(pair<string, CubeRegister*>("eip", eip));
		regMap->insert(pair<string, CubeRegister*>("esp", esp));
		regMap->insert(pair<string, CubeRegister*>("ebp", ebp));
	}

	CubeRegister* queryRegister(string r)
	{
		auto x = regMap->find(r);

		if (x != regMap->end())
			return x->second;

		return NULL;
	}

	SceneNode* getNode()
	{
		return node;
	}

	~CubeRegisterList()
	{
		delete regMap;

		delete eax;
		delete ebx;
		delete ecx;
		delete edx;
		delete eip;
		delete esp;
		delete ebp;
	}	


};

class CubeStack
{
	struct context
	{
		CubeText* text;
		uint val;
	};

	SceneNode* node;
	SceneManager* scnMgr;

	CubeText** cubeAddr;
	context* cubeVal;

	CubeRegister* esp;

	uint stackInitialAddr;  
	uint stackLastAddr;		
	uint stackMaxSize;

public:

	CubeStack(SceneNode* parent_Node, SceneManager* ScnMgr, Vector3 location,
		CubeRegister* Esp, int StackSize = 50, int initial_address = 0x7c00)
	{
		scnMgr = ScnMgr;
		node = parent_Node->createChildSceneNode();
		node->setPosition(location);

		stackInitialAddr = initial_address;
		stackLastAddr = initial_address - StackSize;
		stackMaxSize = StackSize;

		cubeAddr = new CubeText*[StackSize];
		cubeVal = new context[StackSize];

		esp = Esp;
		esp->set_value(initial_address);

		for (int i = 0; i < StackSize; i++)
		{
			Vector3 loc = location;
			loc.y += i * Cubic::size_y;
			loc.x += 8 * Cubic::size_x; 

			cubeAddr[i] = new CubeText(uintValToHexString(initial_address - i), node, ScnMgr, loc);

			Vector3 loc2 = location;
			loc2.y += i * Cubic::size_y;
			loc2.x -= Cubic::delta_x;

			cubeVal[i].text = new CubeText("00000000", node, ScnMgr, loc2);
			cubeVal[i].val = 0;
		}
	}

	~CubeStack()
	{
		uint stackMaxSize = stackInitialAddr - stackLastAddr;

		for (int i = 0; i < stackMaxSize; i++)
			delete cubeAddr[i];
		delete cubeAddr;

		for (int i = 0; i < stackMaxSize; i++)
			delete cubeVal[i].text;
		delete[] cubeVal;

		node->getParentSceneNode()->removeAndDestroyChild(node);
	}

	bool push(uint val)
	{	
		uint stackCurrentAddr = esp->getValue();
		uint i = stackInitialAddr - stackCurrentAddr;

		if (i < stackMaxSize)
		{
			// cubeVal[stackCurrentSize].text != 0 always
			delete cubeVal[i].text;

			Vector3 nodeLoc = node->getPosition();
			CubeText *x = new CubeText(uintValToHexString(val), node, scnMgr);
			x->getSceneNode()->setPosition(nodeLoc.x - Cubic::delta_x,
				nodeLoc.y + i * Cubic::size_y, nodeLoc.z);
	
			cubeVal[i].text = x;
			cubeVal[i].val = val;

			esp->set_value(stackCurrentAddr - 1);
			return true;
		}

		return false;
	}

	bool pop()
	{
		uint stackCurrentAddr = esp->getValue();
		stackCurrentAddr++;

		esp->set_value(stackCurrentAddr);

		return stackInitialAddr - stackCurrentAddr < stackMaxSize;
	}

	CubeText* topText()
	{
		uint stackCurrentAddr = esp->getValue();
		stackCurrentAddr++;

		uint i = stackInitialAddr - stackCurrentAddr;
		return i < stackMaxSize ? cubeVal[i].text : NULL;
	}
	
	uint topVal()
	{
		uint stackCurrentAddr = esp->getValue();
		stackCurrentAddr++;

		uint i = stackInitialAddr - stackCurrentAddr;
		return i < stackMaxSize ? cubeVal[i].val : NULL;
	}

	SceneNode* getNode()
	{
		return node;
	}

};

struct InstructionContext
{
	uint addr;
	string command;
	string arg1;
	string arg2;
};

class CubeInstruction
{
	InstructionContext context;
	SceneNode* node;
	CubeText* cubeText;

public:

	CubeInstruction(SceneNode* parent_Node, SceneManager* ScnMgr,
		InstructionContext con, Vector3 location = Vector3(0,0,0))
	{
		node = parent_Node->createChildSceneNode();
		node->setPosition(location);
		context = con;
		
		string sAddr = uintValToHexString(con.addr);
		string fullCom = sAddr + " " + con.command + " " + con.arg1 + " " + con.arg2;
		cubeText = new CubeText(fullCom, node, ScnMgr);
	}

	SceneNode* getSceneNode()
	{
		return node;
	}

	SceneNode* getCubeTextSceneNode()
	{
		return cubeText->getSceneNode();
	}

	InstructionContext getInstructionContext()
	{
		return context;
	}

	~CubeInstruction()
	{
		delete cubeText;
		node->getParentSceneNode()->removeAndDestroyChild(node);
	}
};

class CubeInstructionStorage
{
	uint orgAddr, espAddr, stackSize;
	std::vector<CubeInstruction*> vStorage;

	CubeInstruction* curInstruction;
	SceneNode* node;
	SceneManager* scnMgr;

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
		int radix;

		printf("Parsing setup section\n");

		pNode = pRoot->first_node("setup");
		pAttr = pNode->first_attribute("org_address");
		strVal = pAttr->value();
		radix = (strVal.back() == 'h' || strVal.back() == 'H') ? 16 : 10;
		orgAddr = stoi(strVal, NULL, radix);

		pNode = pNode->next_sibling();
		pAttr = pNode->first_attribute("esp_address");
		strVal = pAttr->value();
		radix = (strVal.back() == 'h' || strVal.back() == 'H') ? 16 : 10;
		espAddr = stoi(strVal, NULL, radix);

		pNode = pNode->next_sibling();
		pAttr = pNode->first_attribute("stack_size");
		strVal = pAttr->value();
		radix = (strVal.back() == 'h' || strVal.back() == 'H') ? 16 : 10;
		stackSize = stoi(strVal, NULL, radix);

		if (orgAddr > 0xFFFFF000)
			throw "Invalid parameter org_address";
		if (stackSize > espAddr)
			throw "Invalid parameters esp_address, stack_size";

		printf("Setup:\norg: %08X\nesp: %08X\nstack size: %08X\n", orgAddr, espAddr, stackSize);

		///////////// then load all instructions with arguments to vector

		printf("Parsing code section\n");
		int i = 0;
		for (xml_node<> *pNode = pRoot->first_node("command"); pNode; pNode = pNode->next_sibling())
		{
			InstructionContext con;
			con.command = pNode->first_attribute("name")->value();
			con.addr = orgAddr + i;

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

			printf("%08X %s %s %s\n", orgAddr + i, con.command.c_str(), con.arg1.c_str(), con.arg2.c_str());

			Vector3 loc = node->getPosition();
			loc.y -= i * (Cubic::size_y + Cubic::delta_y);

			vStorage.push_back(new CubeInstruction(node, scnMgr, con, loc));
			i++;
		}
	}

public:

	CubeInstructionStorage(string filename, SceneNode* parent_Node, SceneManager* ScnMgr,
		Vector3 location = Vector3(0, 0, 0))
	{
		node = parent_Node->createChildSceneNode();
		node->setPosition(location);
		scnMgr = ScnMgr;

		parseXML(filename);
	}

	~CubeInstructionStorage()
	{
		while (!vStorage.empty())
		{
			CubeInstruction* x = vStorage.back();
			delete x;

			vStorage.pop_back();
		}

		node->getParentSceneNode()->removeAndDestroyChild(node);
	}

	CubeInstruction* getInstruction(uint address)
	{
		if (address - orgAddr < vStorage.size())
			return vStorage[address - orgAddr];

		return NULL;
	}

	bool isLastInstruction(uint address)
	{
		return address - orgAddr == vStorage.size();
	}

	SceneNode* getNode()
	{
		return node;
	}

	uint getStackSize()
	{
		return stackSize;
	}

	uint getEsp()
	{
		return espAddr;
	}

	uint getOrg()
	{
		return orgAddr;
	}
};

//////////////////////// asm commands

class AsmCommand
{

protected:

	CubeRegister* reg1;
	CubeRegister* reg2;

	uint toHex(string argStr)
	{
		int radix = (argStr.back() == 'h' || argStr.back() == 'H') ? 16 : 10;
		return stoi(argStr, NULL, radix);
	}

	void getRegisters(CubeRegisterList* rList, const string& argStr1, const string& argStr2)
	{
		reg1 = rList->queryRegister(argStr1);
		reg2 = rList->queryRegister(argStr2);
	}

public:

	virtual string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2)
	{
		return "Ok";
	}

	virtual uint setupEIP(CubeRegister* eip, bool* reverse)
	{
		uint val = eip->getValue();
		eip->set_value(val + 1);
		*reverse = false;

		return Cubic::delta_y + Cubic::size_y;
	}
};

////////////////////////////////// imlemented asm commands
// mov r1, r2 ; mov r1, d
class AsmMove : public AsmCommand
{
	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);

		if (reg1 != NULL && reg2 != NULL)
		{
			uint val = reg2->getValue();
			reg1->set_value(val);
			return "Ok";
		}

		else if (reg1 != NULL && argStr2 != "")
		{
			uint val = toHex(argStr2);
			reg1->set_value(val);
			return "Ok";
		}

		else
			return "command mov invalid usage";
	}
};
// add r1, r2; add r1, d
class AsmAdd : public AsmCommand
{
	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);

		if (reg1 != NULL && reg2 != NULL)
		{
			uint val = reg2->getValue();
			reg1->set_value(reg1->getValue() + val);
			return "Ok";
		}

		else if (reg1 != NULL && argStr2 != "")
		{
			uint val = toHex(argStr2);
			reg1->set_value(reg1->getValue() + val);
			return "Ok";
		}

		else
			return "command mov invalid usage";
	}
};
// sub r1, r2; sub r1, d
class AsmSub : public AsmCommand
{
	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);

		if (reg1 != NULL && reg2 != NULL)
		{
			uint val = reg2->getValue();
			reg1->set_value(reg1->getValue() - val);
			return "Ok";
		}

		else if (reg1 != NULL && argStr2 != "")
		{
			uint val = toHex(argStr2);
			reg1->set_value(reg1->getValue() - val);
			return "Ok";
		}

		else
			return "command mov invalid usage";
	}
};
// push r; push d
class AsmPush : public AsmCommand
{
	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);

		if (reg1 != NULL || argStr1 != "")
		{
			uint val;
			if (reg1 != NULL)
				val = reg1->getValue();
			else 
				val = toHex(argStr1);
			
			if (!stack->push(val))
				return "Stack overflow";

			return "Ok";
		}

		else
			return "command push invalid usage";
	}
};
// pop r
class AsmPop : public AsmCommand
{
	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);
		
		if (reg1 != NULL)
		{
			uint val = stack->topVal();

			if (!stack->pop())
				return "Stack underflow";

			reg1->set_value(val);
			return "Ok";
		}
		
		else
			return "command pop invalid usage";
	}

};
// call r; call d
class AsmCall : public AsmCommand
{
	uint newPos = 0;

	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);

		if (reg1 != NULL || argStr1 != "")
		{
			if (reg1 != NULL)
				newPos = reg1->getValue();
			else
				newPos = toHex(argStr1);

			// push to stack addr of next instruction
			CubeRegister* eip = rList->queryRegister("eip");
			if (!stack->push(eip->getValue() + 1))
				return "Stack overflow";

			return "Ok";
		}

		else
			return "command call invalid usage";
	}

	virtual uint setupEIP(CubeRegister* eip, bool* rev) override
	{
		uint curPos = eip->getValue();
		eip->set_value(newPos);
		uint delta;

		if (newPos > curPos)
		{
			*rev = false;
			delta = newPos - curPos;
		}

		else
		{
			*rev = true;
			delta = curPos - newPos;
		}

		return (Cubic::delta_y + Cubic::size_y) * delta;
	}
};
// jmp r; jmp d
class AsmJmp : public AsmCommand
{
	uint newPos = 0;

	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		getRegisters(rList, argStr1, argStr2);
		uint delta;

		if (reg1 != NULL || argStr1 != "")
		{
			if (reg1 != NULL)
				delta = reg1->getValue();
			else
				delta = toHex(argStr1);

			CubeRegister* eip = rList->queryRegister("eip");
			newPos = delta + eip->getValue();

			return "Ok";
		}

		else
			return "command jmp invalid usage";
	}

	virtual uint setupEIP(CubeRegister* eip, bool* rev) override
	{
		uint curPos = eip->getValue();
		eip->set_value(newPos);
		uint delta;

		if (newPos > curPos)
		{
			*rev = false;
			delta = newPos - curPos;
		}

		else
		{
			*rev = true;
			delta = curPos - newPos;
		}

		return (Cubic::delta_y + Cubic::size_y) * delta;
	}
};
// ret
class AsmRet : public AsmCommand
{
	uint newPos = 0;

	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		// get return address
		newPos = stack->topVal();
		
		if (!stack->pop())
			return "Stack underflow";

		return "Ok";
	}

	virtual uint setupEIP(CubeRegister* eip, bool* rev) override
	{
		uint curPos = eip->getValue();
		eip->set_value(newPos);
		uint delta;

		if (newPos > curPos)
		{
			*rev = false;
			delta = newPos - curPos;
		}

		else
		{
			*rev = true;
			delta = curPos - newPos;
		}

		return (Cubic::delta_y + Cubic::size_y) * delta;
	}

};

// retn d
class AsmRetn : public AsmCommand
{
	uint newPos = 0;

	string exec(CubeRegisterList* rList, CubeStack* stack,
		const string& argStr1, const string& argStr2) override
	{
		if (argStr1 == "")
			return "Command retn invalid usage";

		// pop few bytes in stack
		uint valToPop = stoi(argStr1);
		if (valToPop % 4 != 0) // 4 bytes
			return "Command retn invalid parameter " + uintValToHexString(valToPop);
		
		CubeRegister* esp = rList->queryRegister("esp");
		esp->set_value(esp->getValue() + valToPop / 4);

		// get return address
		newPos = stack->topVal();

		if (!stack->pop())
			return "Stack underflow";

		return "Ok";
	}

	virtual uint setupEIP(CubeRegister* eip, bool* rev) override
	{
		uint curPos = eip->getValue();
		eip->set_value(newPos);
		uint delta;

		if (newPos > curPos)
		{
			*rev = false;
			delta = newPos - curPos;
		}

		else
		{
			*rev = true;
			delta = curPos - newPos;
		}

		return (Cubic::delta_y + Cubic::size_y) * delta;
	}

};


////////////////////////////////// imlemented asm commands

class AsmCommandList
/*

Class for execution asm commands, supports:

mov
push
pop
call
jmp
ret
retn
add
sub
nop

*/
{
	std::map<string, AsmCommand*> comMap;

public:

	AsmCommandList()
	{
		AsmCommand* mov = new AsmMove();

		comMap.insert(pair<string, AsmCommand*>("mov", new AsmMove()));
		comMap.insert(pair<string, AsmCommand*>("push", new AsmPush()));
		comMap.insert(pair<string, AsmCommand*>("pop", new AsmPop()));
		comMap.insert(pair<string, AsmCommand*>("call", new AsmCall()));
		comMap.insert(pair<string, AsmCommand*>("jmp", new AsmJmp()));
		comMap.insert(pair<string, AsmCommand*>("ret", new AsmRet()));
		comMap.insert(pair<string, AsmCommand*>("retn", new AsmRetn()));
		comMap.insert(pair<string, AsmCommand*>("add", new AsmAdd()));
		comMap.insert(pair<string, AsmCommand*>("sub", new AsmSub()));
		comMap.insert(pair<string, AsmCommand*>("nop", new AsmCommand()));
	}

	~AsmCommandList()
	{
		for (auto& elem : comMap)
		{
			delete elem.second;
		}
	}

	AsmCommand* queryCommand(string com)
	{
		auto x = comMap.find(com);

		if (x != comMap.end())
			return x->second;

		return NULL;
	}
};

/////////////////////// asm commands

class InstructionWalker
{
	AsmCommandList* cList;
	CubeRegisterList* rList;
	CubeStack* pStack;
	CubeInstructionStorage* iStorage;

	///// graphics

	void unmarkInstruction(uint addr)
	{
		SceneNode* xNode = iStorage->getInstruction(addr)->getSceneNode();
		Vector3 scale = xNode->getScale();
		xNode->setScale(scale / 1.5);
	}

	void markInstruction(uint addr)
	{
		SceneNode* xNode = iStorage->getInstruction(addr)->getSceneNode();
		Vector3 scale = xNode->getScale();
		xNode->setScale(scale * 1.5);
	}

	void move(uint val, bool reverse)
	{
		Real delta = (Real)val;

		if (reverse)
			delta *= -1;

		iStorage->getNode()->translate(Vector3(0.0f, delta, 0.0f));
	}

public:

	InstructionWalker(
		AsmCommandList* cList,
		CubeRegisterList* rList,
		CubeStack* pStack,
		CubeInstructionStorage* iStorage
		) : cList(cList), rList(rList), iStorage(iStorage), pStack(pStack)
	{
		CubeRegister* eip = rList->queryRegister("eip");
		eip->set_value(iStorage->getOrg());

		markInstruction(iStorage->getOrg());
	}

	///// exec

	string execCom()
	{
		string comName, r1, r2;

		// get cur instruction address
		CubeRegister* eip = rList->queryRegister("eip");
		uint addr = eip->getValue();

		// get instruction context
		CubeInstruction* x = iStorage->getInstruction(addr);
		InstructionContext con = x->getInstructionContext();
		comName = con.command;
		r1 = con.arg1;
		r2 = con.arg2;

		// unmark current instruction
		unmarkInstruction(addr);

		// execute current instruction
		AsmCommand* asmCom = cList->queryCommand(comName);

		if (asmCom == NULL)
			return "Unsupported instruction " + comName + " on address " + uintValToHexString(addr) + "H";

		string stat = asmCom->exec(rList, pStack, r1, r2);

		if (stat != "Ok")
			return "Execute error on address "+ uintValToHexString(addr) + "H" + " " + stat;

		// change eip value
		// rev indicates is jump up or down
		bool rev;	
		uint valToMove = asmCom->setupEIP(eip, &rev);

		// before moving check if invalid address was passed (jmp call)
		uint uAddr = eip->getValue();
		if (iStorage->getInstruction(uAddr) == NULL)
		{
			// check if it's last instruction
			if (iStorage->isLastInstruction(uAddr))
				return "Debugging finished";

			return "Access Violation on address " + uintValToHexString(uAddr) + "H";
		}

		// move list
		move(valToMove, rev);
		
		// mark current instruction
		markInstruction(eip->getValue());
		return "Ok";
	}
};

////////////////////////////////////////////

class Application
	: public ApplicationContext
	, public InputListener
{
public:
	Application();
	virtual ~Application() 
	{
	}

	void setup();

	bool keyPressed(const KeyboardEvent& evt);

	void free()
	{
		if (cubeStack)
			delete cubeStack;
		if (cubeRegisterList)
			delete cubeRegisterList;
		if (cubeInstructionStorage)
			delete cubeInstructionStorage;
		if (iWalker)
			delete iWalker;
		if (asmCommandList)
			delete asmCommandList;
		if (finalMsg)
			delete finalMsg;
	}

	void setError()
	{
		err = true;
		//getRenderWindow()->getViewport()->setBackgroundColour(
		//getRenderWindow()->getViewport()->setBackgroundColour(ColorValue(200, 0, 0));
	}

	bool is_error()
	{
		return err;
	}

private:

	bool err = false;
	CubeRegisterList *cubeRegisterList = NULL;
	CubeStack* cubeStack = NULL;
	CubeInstructionStorage* cubeInstructionStorage = NULL;
	InstructionWalker* iWalker = NULL;
	AsmCommandList* asmCommandList = NULL;
	CubeText* finalMsg = NULL;

	Ogre::Camera* pCamera;
	Ogre::SceneNode* pCameraNode;
	Ogre::SceneManager* pSceneMgr;

};

class MyFrameListener : public Ogre::FrameListener
/*

This class is need for moving inside 3d space 

Use w,a,s,d for changing position and space for faster movement

*/
{
private:

	Ogre::Camera* _Cam;
	OIS::InputManager* _man;
	OIS::Keyboard* _key;
	OIS::Mouse* _mouse;

public:

	MyFrameListener(Ogre::RenderWindow* win, Ogre::Camera* cam)
	{
		size_t windowHnd = 0;
		std::stringstream windowHndStr;
		_Cam = cam;

		win->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;

		OIS::ParamList pl;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

		_man = OIS::InputManager::createInputSystem(pl);
		_key = static_cast<OIS::Keyboard*> (_man->createInputObject(OIS::OISKeyboard, false));
		_mouse = static_cast<OIS::Mouse*>(_man->createInputObject(OIS::OISMouse, false));
	}

	~MyFrameListener()
	{
		_man->destroyInputObject(_mouse);
		_man->destroyInputObject(_key);
		OIS::InputManager::destroyInputSystem(_man);
	}

	void cameraMove(const Ogre::FrameEvent &evt)
	{
		float cur_movement_speed = 1;
		Ogre::Vector3 translate(0, 0, 0);

		_key->capture();

		if (_key->isKeyDown(OIS::KC_W))
		{
			translate += Ogre::Vector3(0, 0, -10);
		}

		if (_key->isKeyDown(OIS::KC_S))
		{
			translate += Ogre::Vector3(0, 0, 10);
		}

		if (_key->isKeyDown(OIS::KC_A))
		{
			translate += Ogre::Vector3(-10, 0, 0);
		}
		if (_key->isKeyDown(OIS::KC_D))
		{
			translate += Ogre::Vector3(10, 0, 0);
		}

		if (_key->isKeyDown(OIS::KC_SPACE))
		{
			cur_movement_speed *= Cubic::cameraBoost;
		}

		_mouse->capture();
		float _rotX = _mouse->getMouseState().X.rel * evt.timeSinceLastFrame* -1;
		float _rotY = _mouse->getMouseState().Y.rel * evt.timeSinceLastFrame * -1;

		_Cam->yaw(Ogre::Radian(_rotX) / 5.0);
		_Cam->pitch(Ogre::Radian(_rotY) / 5.0);
		_Cam->moveRelative(translate * evt.timeSinceLastFrame * cur_movement_speed);

	}

	bool frameStarted(const Ogre::FrameEvent &evt)
	{
		//Animator.step();
		cameraMove(evt);
		return true;
	}
};

Application::Application()
	: ApplicationContext("OgreTutorialApp")
{
}

void Application::setup()
{
	// do not forget to call the base first
	ApplicationContext::setup();
	addInputListener(this);

	// get a pointer to the already created root
	Root* root = getRoot();
	pSceneMgr = root->createSceneManager();

	// register our scene with the RTSS
	RTShader::ShaderGenerator* shadergen = RTShader::ShaderGenerator::getSingletonPtr();
	shadergen->addSceneManager(pSceneMgr);

	// -- tutorial section start --

	//turnlights
	pSceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));

	Ogre::Light* light1 = pSceneMgr->createLight("Light1");
	light1->setType(Ogre::Light::LT_DIRECTIONAL);

	light1->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	light1->setDirection(Ogre::Vector3(0, -1, -1));


	pCameraNode = pSceneMgr->getRootSceneNode()->createChildSceneNode();

	// create the camera
	pCamera = pSceneMgr->createCamera("myCam");
	pCamera->setNearClipDistance(5); // specific to this sample
	pCamera->setAutoAspectRatio(true);
	pCameraNode->attachObject(pCamera);
	pCameraNode->setPosition(110, 40, 100);
	
	// and tell it to render into the main window
	getRenderWindow()->addViewport(pCamera);

	FrameListener *frameListener = new MyFrameListener(getRenderWindow(), pCamera);
	root->addFrameListener(frameListener);

	Vector3 addrStorage = Vector3(40.0f, 20.0f, 0.0f);
	Vector3 addrStack = Vector3(-5.0f,  20.0f, 30.0f);
	Vector3 addrReg = Vector3(8.0f, 10.0f, 16.2f);

	// initializing model
	printf("Parsing XmlParse\\xml_file.xml\n");
	cubeInstructionStorage = new CubeInstructionStorage("XmlParse\\xml_file.xml",
		pSceneMgr->getRootSceneNode(), pSceneMgr, addrStorage);

	printf("Creating registers\n");
	cubeRegisterList = new CubeRegisterList(pSceneMgr->getRootSceneNode(), pSceneMgr, addrReg);

	printf("Creating stack\n");
	cubeStack = new CubeStack(pSceneMgr->getRootSceneNode(), pSceneMgr, addrStack, 
		cubeRegisterList->queryRegister("esp"), cubeInstructionStorage->getStackSize(), cubeInstructionStorage->getEsp());

	cubeStack->getNode()->yaw(Degree(40));
	cubeRegisterList->getNode()->yaw(Degree(40));

	printf("Initializing asm commands\n");
	asmCommandList = new AsmCommandList();
	iWalker = new InstructionWalker(asmCommandList, cubeRegisterList, cubeStack, cubeInstructionStorage);

	printf("All done\n");
}

bool Application::keyPressed(const KeyboardEvent& evt)
{
	if (evt.keysym.sym == SDLK_ESCAPE)
	{
		getRoot()->queueEndRendering();
	}

	if (!is_error())
	{
		// using 'r' to go through asm code execution
		if (evt.keysym.sym == SDLK_r)
		{
			string stat = iWalker->execCom();

			if (stat != "Ok")
			{
				setError();
				string s = "Program finished with status " + stat;
				printf(stat.c_str());

				Vector3 loc(0, 10, 100);
				finalMsg = new CubeText(stat, pSceneMgr->getRootSceneNode(), pSceneMgr, loc);
				finalMsg->getSceneNode()->yaw(Degree(20));
			}
		}
	}
	
	return true;
}

int main(int argc, char **argv)
{
	try
	{
		Application app;
		app.initApp();
		app.getRoot()->startRendering();
		app.free();
		app.closeApp();
	}

	catch (const std::exception& e)
	{
		printf("Error occurred during execution: %s\n", e.what());
		return 1;
	}

	catch (char *x)
	{
		printf("Parse xml error occured: %s\n", x);
		return 2;
	}

	return 0;
}
