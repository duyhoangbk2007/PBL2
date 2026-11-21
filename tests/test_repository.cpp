
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include "../src/data_access/Repository.h"

using namespace std;

struct TestItem
{
    string id;
    static int alive;

    TestItem(const string& value) : id(value)
    {
        alive++;
    }

    ~TestItem()
    {
        alive--;
    }
};

int TestItem::alive = 0;

string getTestId(TestItem* item)
{
    return item->id;
}

TestItem* parseTestItem(const string& line)
{
    if (line == "INVALID")
    {
        throw runtime_error("Du lieu loi");
    }

    return new TestItem(line);
}

string serializeTestItem(TestItem* item)
{
    return item->id;
}

void writeTestFile(const string& content)
{
    ofstream file("test_repository_data.txt");
    file << content;
}

void check(bool condition, const string& name)
{
    if (!condition)
    {
        throw runtime_error("FAILED: " + name);
    }

    cout << "PASSED: " << name << '\n';
}

int main()
{
    try
    {
        const string path = "test_repository_data.txt";

        writeTestFile("A\nB\n");

        {
            Repository<TestItem> repo(
                path,
                getTestId,
                parseTestItem,
                serializeTestItem
            );

            check(repo.load(), "Load hop le");
            check(repo.count() == 2, "Dem doi tuong");
            check(TestItem::alive == 2, "Ownership sau load");
            check(!repo.load(), "Khong load hai lan");

            TestItem* duplicate = new TestItem("A");

            check(!repo.add(duplicate), "Tu choi ID trung");
            delete duplicate;

            TestItem* item = new TestItem("C");

            check(repo.add(item), "Them doi tuong");
            check(!repo.add(item), "Tu choi trung con tro");

            check(repo.count() == 3, "Dem sau add");
            check(repo.save(), "Save thanh cong");

            repo.clear();

            check(repo.count() == 0, "Clear danh sach");
            check(!repo.isLoaded(), "Reset loaded");
            check(TestItem::alive == 0, "Khong ro ri bo nho");
        }

        writeTestFile("A\nA\n");

        {
            Repository<TestItem> repo(
                path,
                getTestId,
                parseTestItem,
                serializeTestItem
            );

            bool threw = false;

            try
            {
                repo.load();
            }
            catch (const runtime_error&)
            {
                threw = true;
            }

            check(threw, "Phat hien ID trung");
            check(repo.count() == 0, "Rollback ID trung");
            check(TestItem::alive == 0, "Don bo nho ID trung");
        }

        writeTestFile("A\nINVALID\n");

        {
            Repository<TestItem> repo(
                path,
                getTestId,
                parseTestItem,
                serializeTestItem
            );

            bool threw = false;

            try
            {
                repo.load();
            }
            catch (const runtime_error&)
            {
                threw = true;
            }

            check(threw, "Phat hien parse loi");
            check(repo.count() == 0, "Rollback parse loi");
            check(TestItem::alive == 0, "Don bo nho parse loi");
        }

        cout << "\nALL TESTS PASSED\n";
    }
    catch (const exception& e)
    {
        cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
