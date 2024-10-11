#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <set>
#include <algorithm>
#include <chrono>

using namespace std;

class Transaction {
public:
    string tr_id, source_ac, dest_ac, type;
    float amount;
    time_t timestamp;

    Transaction(string id, string source, string dest, float amt, string trans_type)
        : tr_id(id), source_ac(source), dest_ac(dest), amount(amt), type(trans_type) {
        timestamp = time(0);
    }

    void execute() {
        // Perform necessary operations to complete the transaction.
        cout << "Transaction Executed: " << tr_id << " Type: " << type << endl;
    }
};

class Account {
protected:
    string account_number;
    float balance;
    vector<Transaction> tr_his;

public:
    Account(string acc_num, float cur_bal)
        : account_number(acc_num), balance(cur_bal) {}

    virtual void deposit(float amount) {
        balance += amount;
        tr_his.emplace_back("Deposit", "", account_number, amount, "deposit");
        cout << "Deposit Successful: $" << amount << " to account " << account_number << endl;
    }

    virtual void withdraw(float amount) {
        if (balance >= amount) {
            balance -= amount;
            tr_his.emplace_back("Withdraw", account_number, "", amount, "withdrawal");
            cout << "Withdrawal Successful: $" << amount << " from account " << account_number << endl;
        } else {
            cout << "Insufficient Funds" << endl;
        }
    }

    void transfer(float amount, Account& dest_account) {
        if (balance >= amount) {
            balance -= amount;
            dest_account.deposit(amount);
            tr_his.emplace_back("Transfer", account_number, dest_account.account_number, amount, "transfer");
            cout << "Transfer Successful: $" << amount << " from " << account_number << " to " << dest_account.account_number << endl;
        } else {
            cout << "Transfer Failed: Insufficient Funds" << endl;
        }
    }

    float get_balance() const {
        return balance;
    }

    string get_account_number() const {
        return account_number;
    }

    const vector<Transaction>& get_tr_his() const {
        return tr_his;
    }

    virtual void apply_interest() {}
};

class SavingsAccount : public Account {
private:
    float interest_rate;

public:
    SavingsAccount(string acc_num, float cur_bal, float rate)
        : Account(acc_num, cur_bal), interest_rate(rate) {}

    void apply_interest() override {
        balance += balance * interest_rate;
        cout << "Interest Applied: New Balance = $" << balance << " for account " << account_number << endl;
    }
};

class CheckingAccount : public Account {
private:
    float overdraft_limit;

public:
    CheckingAccount(string acc_num, float cur_bal, float overdraft)
        : Account(acc_num, cur_bal), overdraft_limit(overdraft) {}

    void withdraw(float amount) override {
        if (balance + overdraft_limit >= amount) {
            balance -= amount;
            tr_his.emplace_back("Withdraw with Overdraft", account_number, "", amount, "withdrawal");
            cout << "Withdrawal with Successful: $" << amount << " from account " << account_number << endl;
        } else {
            cout << "Withdrawal Failed: Exceeds Limit" << endl;
        }
    }
};

class Customer {
private:
    string cust_id, name;
    map<string, Account*> accounts;

public:
    Customer() : cust_id(""), name("") {}

    Customer(string id, string customer_name) : cust_id(id), name(customer_name) {}

    void add_account(Account* account) {
        accounts[account->get_account_number()] = account;
    }

    Account* get_account(string acc_num) {
        return accounts[acc_num];
    }

    string get_id() const { return cust_id; }
    string get_name() const { return name; }
};

class Bank {
private:
    map<string, Customer> customers;
    map<string, Account*> accounts;  // Index by account number
    vector<Transaction> transactions;

public:
    void add_customer(Customer customer) {
        customers[customer.get_id()] = customer;
    }

    void add_account(Account* account, string cust_id) {
        accounts[account->get_account_number()] = account;  // Use account number as key
        customers[cust_id].add_account(account);
    }

    void process_transaction(Transaction transaction) {
        transaction.execute();
        transactions.push_back(transaction);
    }

    vector<Transaction> get_transactions_by_amount(float min_amount, float max_amount) {
        vector<Transaction> result;
        for (auto& transaction : transactions) {
            if (transaction.amount >= min_amount && transaction.amount <= max_amount) {
                result.push_back(transaction);
            }
        }
        return result;
    }

    const vector<Transaction>& get_all_transactions() const {
        return transactions;
    }
};

class FraudDetectionSystem {
private:
    set<string> blacklisted_accounts;
    vector<Transaction> flagged_transactions;

public:
    void monitor_transactions(Bank& bank) {
        for (const auto& transaction : bank.get_all_transactions()) {
            if (transaction.amount > 10000) {
                flag_transaction(transaction);
            }
        }
        detect_rapid_transactions(bank);
    }

    void flag_transaction(const Transaction& transaction) {
        flagged_transactions.push_back(transaction);
        cout << "Transaction Flagged: " << transaction.tr_id << " Amount: $" << transaction.amount << endl;
    }

    void detect_rapid_transactions(Bank& bank) {
        auto now = chrono::system_clock::now();
        auto one_minute_ago = now - chrono::minutes(1);
        int transaction_count = 0;

        for (const auto& transaction : bank.get_all_transactions()) {
            if (transaction.timestamp >= one_minute_ago.time_since_epoch().count()) {
                transaction_count++;
            }
        }

        if (transaction_count > 3) {
            cout << "Rapid transactions detected! More than 3 transactions within a minute." << endl;
        }
    }

    const vector<Transaction>& get_flagged_transactions() const {
        return flagged_transactions;
    }
};

int main() {
    // Setup bank and system
    Bank bank;
    FraudDetectionSystem fraud_system;

    // Create customers and accounts
    Customer alice("CUST1001", "Alice Smith");
    SavingsAccount alice_savings("SA123", 10000.00, 0.02);
    alice.add_account(&alice_savings); 

    Customer bob("CUST1002", "Bob Johnson");
    CheckingAccount bob_checking("CA456", 5000.00, 1000.00);
    bob.add_account(&bob_checking);

    bank.add_customer(alice);
    bank.add_customer(bob);

    Transaction deposit1("TXN001", "SA123", "", 2000.00, "deposit");
    bank.process_transaction(deposit1);
    alice_savings.deposit(2000.00);

    Transaction withdraw1("TXN002", "CA456", "", 6000.00, "withdrawal");
    bank.process_transaction(withdraw1);
    bob_checking.withdraw(6000.00);

    Transaction transfer1("TXN003", "SA123", "CA456", 3000.00, "transfer");
    bank.process_transaction(transfer1);
    alice_savings.transfer(3000.00, bob_checking);

    CheckingAccount bob_new_account("CA789", 0.00, 0.00);
    Transaction transfer2("TXN004", "CA456", "CA789", 15000.00, "transfer");
    bank.process_transaction(transfer2);
    bob_checking.transfer(15000.00, bob_new_account); // Transfer from Bob's account (should fail)

    for (int i = 0; i < 5; i++) {
        Transaction transfer3("TXN00" + to_string(i + 5), "SA123", "CA456", 500.00, "transfer");
        bank.process_transaction(transfer3);
        alice_savings.transfer(500.00, bob_checking);
    }

    // Apply interest to Alice's savings account
    alice_savings.apply_interest();

    // Fraud detection monitoring
    fraud_system.monitor_transactions(bank);

  //Histroy
    cout << "\nAlice's Transaction History:" << endl;
    for (const auto& txn : alice_savings.get_tr_his()) {
        cout << "Transaction ID: " << txn.tr_id << ", Amount: $" << txn.amount << ", Type: " << txn.type << endl;
    }
}