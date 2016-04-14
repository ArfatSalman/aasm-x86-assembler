#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <regex>
#include <cctype>
#include <sstream>
#include <map>

using namespace std;

short LC = 0x0100;
bool DEBUG = false;

int err_exit(string msg="")
{
    cout<<endl<<msg;
    exit(1);
}

string hexstr_to_str(string hex)
{
    if (hex.length() % 2 )
        hex = '0'+ hex;
    int len = hex.length();
    string newString;
    for(int i=0; i< len; i+=2)
    {
        string byte = hex.substr(i,2);
        char chr = (char) (int)strtol(byte.c_str(), NULL, 16);
        newString.push_back(chr);
    }
    return newString;
}

string binstr_to_hexstr(string bin)
{
    stringstream res;
    res << hex <<(int)strtol(bin.c_str(),NULL,2);
    return res.str();
}

string hex_to_bin(string hex)
{
    if (hex.find("h") != string::npos)
        hex.pop_back();
    string str = "";
    for (unsigned i = 0; i < hex.length(); ++i)
    {
        switch(hex.at(i))
        {
            case '1': str += "0001"; break;
            case '2': str += "0010"; break;
            case '3': str += "0011"; break;
            case '4': str += "0100"; break;
            case '5': str += "0101"; break;
            case '6': str += "0110"; break;
            case '7': str += "0111"; break;
            case '8': str += "1000"; break;
            case '9': str += "1001"; break;
            case 'A':
            case 'a': str += "1010"; break;
            case 'B':
            case 'b': str += "1011"; break;
            case 'C':
            case 'c': str += "1100"; break;
            case 'D':
            case 'd': str += "1101"; break;
            case 'E':
            case 'e': str += "1110"; break;
            case 'F':
            case 'f': str += "1111"; break;
            default:
                err_exit("hex_to_bin: Not a valid hex");
        }
    }
    return str;
}

string short_to_hex(short val)
{
    stringstream ss;
    ss<< hex << val; // short decimal_value
    return ss.str();
}

bool is_digits(const string &str)
{
    return str.find_first_not_of("0123456789") == string::npos;
}

bool is_hex(const string &str)
{
    return str.find_first_not_of("0123456789aAbBcCdDeEfF") == string::npos;
}

bool is_register(const string reg)
{
    regex re("ah|al|bh|bl|ch|cl|dh|dl|ax|bx|cx|dx|bp|si|di|sp");
    smatch match;
    try
    {
        if (regex_search(reg, match, re))
            return true;

    } catch (regex_error &e)
    {
        err_exit("Regex Error. Quitting ...");
    }
    return false;
}

bool is_8(string reg)
{
    regex re_8("ah|al|bh|bl|ch|cl|dh|dl");
    regex re_16("ax|bx|cx|dx|bp|si|di|sp");
    smatch match;

    try{
        if (regex_search(reg, match, re_8))
            return true;

    } catch (regex_error &e)
    {
        err_exit("Regex Error!");
    }

    return false;
}

bool is_mnemonic(const string opc)
{
    regex re("mov|int|ret");
    smatch match;
    try
    {
        if (regex_search(opc, match, re))
        {
            return true;
        }

    } catch (regex_error &e)
    {
        err_exit("Regex Error. Quitting ...");
    }
    return false;
}

bool is_imm_data(const string data)
{
    unsigned int len = data.length();
    if ((data.find("'") != string::npos) && len < 4)
        return true;
    else if (data.at(data.length()-1) == 'h')
        return true;
    return false;
}

class Reg {
    map<string, string> coll;
    map<string, string>::iterator pos;

public:
    Reg()
    {
        coll.insert({"al", "000"});
        coll.insert({"ax", "000"});
        coll.insert({"cl", "001"});
        coll.insert({"cx", "001"});
        coll.insert({"dl", "010"});
        coll.insert({"dx", "010"});
        coll.insert({"bl", "011"});
        coll.insert({"bx", "011"});
        coll.insert({"ah", "100"});
        coll.insert({"sp", "100"});
        coll.insert({"ch", "101"});
        coll.insert({"bp", "101"});
        coll.insert({"dh", "110"});
        coll.insert({"si", "110"});
        coll.insert({"bh", "111"});
        coll.insert({"di", "111"});
    }

    string get_reg_value(string reg)
    {
        pos = coll.find(reg);
        if (pos != coll.end())
        {
            return pos->second;
        }
        err_exit("Error while fetching the values of Register.");
        return NULL;
    }
} reg;

class Symbol {
    string name;
    string type; // Such as Variable, Label, etc.
    short location; // offset from the start of the section
    short size; // size of the symbol in bytes
    short section_id; // Stores the section_id
    bool is_global;

public:
    Symbol(string n, string t, short l, short s, short sec = -1, bool is_g = false)
    {
        name = n;
        type = t;
        location = l;
        size = s;
        section_id = sec;
        is_global = is_g;
    }

    string get_type()
    {
        return type;
    }

    short get_size()
    {
        return size;
    }

    short get_loc()
    {
        return location;
    }

    string getName()
    {
        return name;
    }

    void display()
    {
        cout<<name<<'\t'<<type<<'\t'<<location<<'\t'<<size<<'\t'<<section_id<<'\t'<<is_global<<endl;
    }
};

class SYMTAB {
    map<string, Symbol> coll;
    map<string, Symbol>::iterator pos;

public:
    void insert(const string name, const Symbol temp)
    {
        pos = coll.find(name);
        if (pos == coll.end())
        {
            coll.insert({name, temp});
            return;
        }
        else
        {
            cout<<endl<<name<<" already defined.";
            exit(1);
        }
    }

    Symbol valid_label(string label)
    {
        pos = coll.find(label);
        if (pos != coll.end())
            return pos->second;
        err_exit("The label has not been defined yet.");
    }

    void display()
    {
        for (pos = coll.begin(); pos != coll.end(); ++pos)
        {
            Symbol temp = pos->second;
            temp.display();
        }
    }
} sym;

class Mnemonic {
    string mnemonic;
    short size;
    string opcode;

public:
    Mnemonic(string mne, int s, string op)
    {
        mnemonic = mne;
        size = s;
        opcode = op;
    }

    short get_size()
    {
        return size;
    }
    string get_opcode()
    {
        return opcode;
    }
    void display()
    {
        cout<<mnemonic<<'\t'<<size<<'\t'<<opcode<<endl;
    }
};

class MOT {
    map<string, Mnemonic> coll;
    map<string, Mnemonic>::iterator pos;

public:
    MOT()
    {
        coll.insert({"mov_reg_reg", Mnemonic("mov", 2, "100010")});
        coll.insert({"mov_reg_imm8", Mnemonic("mov", 2, "1011")});
        coll.insert({"mov_reg_imm16", Mnemonic("mov", 3, "1011")});
        coll.insert({"mov_reg_mem", Mnemonic("mov", 4, "100010")});
        coll.insert({"mov_acc_mem", Mnemonic("mov", 3, "101000")});
        coll.insert({"int", Mnemonic("int", 2, "11001101")});
        coll.insert({"ret", Mnemonic("ret", 1, "11000011")});
        coll.insert({"lea", Mnemonic("lea", 3, "10001101")});
    }

    short get_size_of(string mnemomic)
    {
        pos = coll.find(mnemomic);
        if (pos != coll.end())
        {
            Mnemonic temp = pos->second;
            return temp.get_size();
        }
        err_exit("Can't find the OPCODE.");
        return 0;
    }

    string get_opcode(string mnemomic)
    {
        pos = coll.find(mnemomic);
        if (pos != coll.end())
        {
            Mnemonic temp = pos->second;
            return temp.get_opcode();
        }
        err_exit("Can't find the OPCODE.");
        return NULL;
    }

    void display()
    {
        for (pos = coll.begin(); pos != coll.end(); ++pos)
        {
            Mnemonic temp = pos->second;
            temp.display();
        }
        return;
    }
} mot;

class Error {
    string line;

public:
    void set_line (string li)
    {
        line = li;
    }

    void prnt_err(string msg)
    {
        cout<<"\nIn line "<<line;
        cout<<msg;
        exit(1);
    }

    void reg_match(string reg1, string reg2)
    {
        if (is_register(reg1) && is_register(reg2))
        {
            if ((is_8(reg1) && !is_8(reg2)) || (!is_8(reg1) && is_8(reg2)) )
            {
                cout<<endl<<reg1<<" and "<<reg2<<"do not match in size.";
                prnt_err("");
            }
        }
        else
        {
            cout<<endl<<reg1<<" and "<<reg2<<"are not Intel recognized registers.\n";
            exit(1);
        }
    }

    void in_data(string datatype, string init)
    {
        if (datatype == "db")
        {
            if (init.find("'") == string::npos)
            {
                if (!is_hex(init.substr(0,init.length()-1)) || init.back() != 'h')
                    prnt_err("The initializer type can't be ascertained. Put h for Hex values");

            }
            else
                if (init.length() > 3)
                    prnt_err("The initializer overflows the variable/register length. Use single character.");
        }
        else if (datatype == "dw")
        {
            if (init.find("\"") == string::npos)
            {
                 if (!is_hex(init.substr(0,init.length()-1)) || init.back() != 'h')
                    prnt_err("The initializer is not an immediate data or it overflows the size of the variable.");
                 else if (init.length() > 5 )
                    prnt_err("The initializer overflows the variable/register length.");
            }
        }
    }

    void pseudo_directive(string label, string datatype, string init)
    {
        if (label == "")
            prnt_err("Label cannot be Null or undefined.");
        else if (is_register(label))
            prnt_err("Label can't be the same as registers. Use different Label.");

        in_data(datatype, init);
    }

    void in_instructions(string label, string mnemonic, string operand1, string operand2)
    {
        if (mnemonic == "ret")
            return;
        if (!is_mnemonic(mnemonic))
            prnt_err("The Mnemonic is not recognized.");
        else if (operand1 != "" && operand1.back() != ',')
            prnt_err("Syntax Error: Use a comma after the first operand.");
        else if (is_register(operand2))
            reg_match(operand1, operand2);
        else if (is_imm_data(operand2))
        {
            if (mnemonic == "int")
                in_data("db", operand2);
            if (is_8(operand1))
                in_data("db", operand2);
            else
                in_data("dw", operand2);
        }
    }
} err;

string identify_opcode_type(string opcode, string operand1, string operand2)
{
    if (opcode == "mov")
    {
        if (is_register(operand1))
        {
            if (is_register(operand2))
                return "mov_reg_reg";
            else if (is_imm_data(operand2))
            {
                if (is_8(operand1))
                    return "mov_reg_imm8";
                else
                    return "mov_reg_imm16";
            }
            else if (operand1 == "al" || operand1 == "ax")
                return "mov_acc_mem";
            else
                return "mov_reg_mem";
        }
        else
            err_exit("Unidentified Register.");
    } // FOR movs
    else if (opcode == "int")
        return "int";
    else if (opcode == "ret")
        return "ret";
    else if (opcode == "lea")
        return "lea"
    else
        err_exit("OPCODE NOT DEFINED.");
    return NULL;
}

void fill_symbol_table(string var, string datatype, string init)
{
    if (DEBUG)
    {
        cout<<"\nFilling Symbol Table with -  ";
        cout<<"\nLabel: "<<var<<"\tData type: "<<datatype<<"\t Initializer: "<<init;
    }

    short size = 0;
    if (datatype == "dw")
    {
        size = 2;
        if (init.find("\"") != string::npos)
        {
            size = 0;
            size += init.length() - 2; // Since there are 2 "'s
        }
    }
    else if (datatype == "db")
        size = 1;

    Symbol temp(var, datatype, LC, size);
    sym.insert(var,temp);
    LC += size;
}

void fill_symbol_table(string label, string opcode, string operand1, string operand2)
{
    if (label != "")
    {
        label.pop_back(); // Removing : from the labels
        Symbol temp(label, "Label", LC, 0);
        sym.insert(label, temp);
    }
    if (opcode != "")
    {
        string str = identify_opcode_type(opcode, operand1, operand2);
        LC = LC + mot.get_size_of(str);
    }
}

class CodeGenerator{
    string CODE;
    string opcode;
    string w;
    string d;
    string oo;
    string rrr;
    string mmm;
    string disp;
    string imm_data;
    /*
     00 - if mmm = 110, a displacement follows the opcode; otherwise no displacement
     01 - An 8-bit signed disp follows the opcode
     10 - a 16 or 32 bit disp follows the opcode
     11 - the mmm is a register instead of an addressing mode
     */
public:

    void generate_code(string opcode, string d = "", string w = "", string oo = "", string rrr="",
                       string mmm="", string imm_data = "", string disp="")
                       {
                           CODE.clear();

                           string temp = opcode;

                            if (d != "")
                                temp += d;
                            if (w != "")
                                temp += w;
                            if (oo != "");
                                temp += oo;
                            if (rrr != "")
                                temp += rrr;
                            if (mmm != "")
                                temp += mmm;
                           if (imm_data != "")
                           {
                                if (imm_data.find("'") != string::npos)
                                    imm_data = short_to_hex(((char)imm_data.at(1)));
                                else
                                {
                                    imm_data.pop_back(); // To remove h
                                    imm_data = hexstr_to_str(imm_data);
                                    if (imm_data.length() > 1)
                                        swap(imm_data.at(0), imm_data.at(1));
                                }
                           }
                            if (disp != "")
                            {
                                disp = hexstr_to_str(disp);
                                swap(disp.at(0), disp.at(1));
                            }

                           temp = binstr_to_hexstr(temp);
                           if (DEBUG)
                               cout<<"\nThe generated hex is: "<<temp;
                           temp = hexstr_to_str(temp) + disp + imm_data;
                           CODE = temp;
                           put_code();
                       }

    void put_code(string filename="a.com")
    {
        ofstream obj_file(filename, ios::binary|ios::app);

        if (!obj_file.is_open())
            err_exit("File can't be created.");
        else
        {
            if (DEBUG)
                cout<<"\ntellp:<<"<<obj_file.tellp()<<endl<<CODE;
            obj_file.write(CODE.c_str(), CODE.length());
            CODE.clear();
        }
        obj_file.close();
    }
    void lea(string type, string operand1, string operand2)
    {
        // Opcode - 10001101 oorrrmmm disp
        if (is_8(operand1))
            err.prnt_err("LEA cannot store data on 8 bit register.");

        opcode = mot.get_opcode(type);
        d = "", w = "", oo = "", rrr = "", mmm = "", imm_data = "";
        Symbol label = sym.valid_label(operand2);

        disp = short_to_hex(label.get_loc());

        generate_code(opcode, d, w, oo, rrr, mmm, imm_data, disp);
    }

    void mov_reg_mem(string type, string operand1, string operand2)
    {
        Symbol label = sym.valid_label(operand2);

        if ((is_8(operand1) && label.get_type() != "db") || (!is_8(operand1) && label.get_type() != "dw"))
        {
            cout<<operand1<<" and "<<label.getName()<<"do not match in size.";
            exit(1);
        }

        opcode = mot.get_opcode(type);
        d = "1";
        w = "0";
        if (!is_8(operand1))
            w = "1";
        oo = "00";
        rrr = reg.get_reg_value(operand1);
        mmm = "110";
        // Counting the displacement
        disp = short_to_hex(label.get_loc());

        // Opcode - 101000dw disp for ACC to mem
        // opcode - 100010dw oorrrmmm disp
        if (operand1 == "al" || operand1 == "ax")
            generate_code(opcode, "0", w, "", "","","",disp);
        else
            generate_code(opcode, d, w, oo, rrr, mmm, "", disp);
    }

    void mov_acc_mem(string type, string operand1, string operand2)
    {
        mov_reg_mem(type,operand1,operand2);
    }

    void mov_reg_reg(string type, string operand1, string operand2)
    {
        // OPCODE is - 100010dw oorrrmmm disp
        opcode = mot.get_opcode(type);
        d = "1";
        w = "0";
        if (!is_8(operand1))
            w = "1";
        oo = "11";
        rrr = reg.get_reg_value(operand1);
        mmm = reg.get_reg_value(operand2);

        generate_code(opcode, d, w, oo, rrr, mmm);
        return;
    }

    void mov_reg_imm(string type, string operand1, string operand2)
    {
        // Opcode 1011wrrr data ( of 1 or 2 bytes)
        opcode = mot.get_opcode(type);
        d = "";
        w = "0";
        if (!is_8(operand1))
        {
            w = "1";
            //Padding data for 2 bytes
            unsigned len = operand2.length();
            if (len == 2)
                operand2 = "000" + operand2;
            else if (len == 3)
                operand2 = "00" + operand2;
            else if (len == 4)
                operand2 = "0" + operand2;
        }
        oo = "";
        rrr = reg.get_reg_value(operand1);
        mmm = "";
        imm_data = operand2;

        generate_code(opcode, d, w, oo, rrr, mmm, imm_data);
    }

    void int_(string type, string operand1, string operand2)
    {
        // OPCODE - 11001101 type
        opcode = mot.get_opcode(type);
        d = "", w ="", oo = "", rrr="", mmm = "";
        generate_code(opcode, d, w, oo, rrr, mmm, operand2);
        put_code();
    }

    void ret(string type, string operand1, string operand2)
    {
        if (operand1 != "" && operand2 != "")
            err_exit("ret does not take operands.");

        // OPCODE - 11000011
        opcode = mot.get_opcode(type);
        generate_code(opcode);
    }

    void pseudo_opcode(string datatype, string val)
    {
        if (DEBUG == true)
        {
            cout<<"\nCodeGenerator->pseudo_opcode: \n";
            cout<<datatype<<'\t'<<val;
        }

        if (datatype == "db")
        {
            if (val.find("'") != string::npos)
                CODE += val.substr(1,val.length()-2);
            else
            {
                val.pop_back();
                if (val.length() < 3)
                    CODE += hexstr_to_str(val);
                else
                    err_exit("DB can only take a byte.");
            }
        }
        else if (datatype == "dw")
        {
            if (val.find("\"") != string::npos)
            {
                // Because we are removing two chars("")
                char ending = 0x00;
                CODE += (val.substr(1,val.length()-2)+ending);
            }
            else if (val.length() > 5)
                err_exit("DW can only take 2 bytes.");
            else
            {
                val.pop_back();
                CODE += hexstr_to_str(val);

                if (CODE.length() == 1)
                    CODE = CODE + char(0x00);
                else if (CODE.length() == 2)
                    swap(CODE.at(0), CODE.at(1));
            }
        }
        else
            err_exit("Unidentified data type.");

        put_code();
    }

    void instructions(string label, string opcode, string operand1, string operand2)
    {
        string type = identify_opcode_type(opcode, operand1, operand2);
        if (type == "mov_reg_reg")
            mov_reg_reg(type, operand1, operand2);
        else if (type == "mov_reg_imm8")
            mov_reg_imm(type, operand1, operand2);
        else if (type == "mov_reg_imm16")
            mov_reg_imm(type, operand1, operand2);
        else if (type == "mov_reg_mem")
            mov_reg_mem(type, operand1, operand2);
        else if (type == "mov_acc_mem")
            mov_acc_mem(type, operand1, operand2);
        else if (type == "int")
            int_(type, operand1, operand2);
        else if (type == "ret")
            ret(type, operand1, operand2);
        else if (type == "lea");
            lea(type, operand1, operand2);
        else
            err_exit("Instructions: Can't map instructions");
    }

} gen;

void parse_line(const string line, int pass = 1)
{
    string label = "";
    string mnemonic = "";
    string operand1 = "";
    string operand2 = "";

    regex re("dw|db");
    regex re_blank("^\\s*$");
    regex re_label("^\\s*([a-zA-Z0-9]+:)\\s*$");
    regex re_ins("^\\s*([a-zA-Z0-9]+:)?\\s*([a-z]{3})\\s+([a-z]+,)?\\s*((')?[a-zA-Z0-9]+(')?|[0-9]+)?\\s*$");
    regex re_pot("^\\s*([a-zA-Z0-9]+)\\s*(db|dw)\\s*((\"|\')?([a-zA-Z0-9 $]+)(\"|\')?)\\s*");

    smatch match;

    try
    {
        if (DEBUG)
        {
            cout<<"\nWhile parsing the line: "<<line;
            cout<<"\nThese parameters were found - ";
        }
        if (regex_search(line, match, re_blank))
            return;
        if (regex_search(line, match, re_label))
            fill_symbol_table(match[1],"","","");
        if (regex_search(line, match, re))
        {
            // Matched a Pseudo Directive
            if (regex_search(line, match, re_pot))
            {
                err.set_line(line);
                if (DEBUG)
                    cout<<"\nLabel - "<<match[1]<<"\tdatatype - "<<match[2]<<" and initializer - "<<match[3];
                if (pass == 1)
                    fill_symbol_table(match[1], match[2], match[3]);
                else
                {
                    err.pseudo_directive(match[1], match[2], match[3]);
                    gen.pseudo_opcode(match[2], match[3]);
                }
            }
            else
            {
                cout<<endl<<line<<" can't be properly matched to known data types. Program will exit ...";
                err_exit();
            }
        }
        else
        {
            if (regex_search(line, match, re_ins))
            {
                err.set_line(line);
                label = match[1];
                mnemonic = match[2];
                operand1 = match[3];
                operand2 = match[4];

                if (DEBUG)
                    cout<<"\nLabel: "<<label<<"\tMnemonic: "<<mnemonic<<"\tOp1: "<<operand1<<"\tOp2: "<<operand2;
                if (pass == 1)
                {
                    if (operand1 != "")
                        operand1.pop_back(); // To remove the , from first operand
                    fill_symbol_table(label, mnemonic, operand1, operand2);
                }
                else
                {
                    err.in_instructions(label, mnemonic, operand1, operand2);
                    if (operand1 != "")
                        operand1.pop_back(); // To remove the , from first operand
                    gen.instructions(label, mnemonic, operand1, operand2);
                }
            }
            else
            {
                cout<<endl<<line<<" can't be tokenized under the current system. Please make sure to use";
                cout<<"\nIntel specified Mnemonics and operands. Program will exit ...";
                err_exit();
            }
        }
    } catch (regex_error &e)
    {
        err_exit("Regex Error. Quitting ...");
    }
    return;
}

int pass_1(string filename)
{
    string line;

    ifstream asm_file(filename);
    if (!asm_file.is_open())
        err_exit("File can't be read");
    else
    {
        while(getline(asm_file,line))
        {
            if (DEBUG)
                cout<<"Line to be parsed is: "<<line<<endl;
            if (line == "\n" || line == "")
                continue;
            line = line+"\n";
            parse_line(line);
        }
    }
    asm_file.close();
    return 0;
}

void pass_2(string filename)
{
    LC = 0;
    string line;
    //Removing the previous file if it exist.
    if (ifstream("a.com"))
        remove("a.com");

    ifstream asm_file(filename);
    if (!asm_file.is_open())
        err_exit("File can't be read");
    else
    {
        while(getline(asm_file,line))
        {
            if (DEBUG)
                cout<<"Line to be converted is: "<<line<<endl;
            if (line == "\n" || line ==" " || line=="")
                continue;
            parse_line((line+"\n"), 2);
        }
    }
    asm_file.close();
}

int main()
{
    pass_1("a.asm");
    sym.display();
    pass_2("a.asm");
    system("pause");
}

