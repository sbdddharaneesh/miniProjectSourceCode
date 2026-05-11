#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BANK_PASSWORD "secure@atm"
#define ADMIN_PASSWORD "admin@001"
#define DAILY_WITHDRAW_LIMIT 30000

// ================= CUSTOMER STRUCTURE =================
typedef struct Customer
{
    int accountNumber;

    char name[30];

    char accountCategory[20];

    double balance;

    int atmPin;

    int accountStatus;

    double todayWithdraw;

    char withdrawDate[20];

    char createdOn[40];

    char debitCard[25];

    struct Customer *next;

} Customer;

// ================= TRANSACTION STRUCTURE =================
typedef struct
{
    int accNo;

    char transactionType[30];

    double amount;

    char dateTime[40];

} Transaction;

// ================= GLOBAL HEAD POINTER =================
Customer *head = NULL;

// ================= DATE & TIME =================
void getDateTime(char *buffer)
{
    time_t t = time(NULL);

    struct tm *tmInfo = localtime(&t);

    strftime(buffer,
             40,
             "%d-%m-%Y %H:%M:%S",
             tmInfo);
}

// ================= CURRENT DATE =================
void getTodayDate(char *buffer)
{
    time_t t = time(NULL);

    struct tm *tmInfo = localtime(&t);

    strftime(buffer,
             20,
             "%d-%m-%Y",
             tmInfo);
}

// ================= GENERATE CARD NUMBER =================
void createCardNumber(char *card)
{
    sprintf(card,
            "%04d-%04d-%04d-%04d",
            rand() % 10000,
            rand() % 10000,
            rand() % 10000,
            rand() % 10000);
}

// ================= SAVE TRANSACTION =================
void saveTransaction(int accNo,
                     char type[],
                     double amount)
{
    FILE *fp;

    fp = fopen("transactionData.dat", "ab");

    if (fp == NULL)
    {
        printf("Transaction file error\n");
        return;
    }

    Transaction t;

    t.accNo = accNo;

    strcpy(t.transactionType, type);

    t.amount = amount;

    getDateTime(t.dateTime);

    fwrite(&t,
           sizeof(Transaction),
           1,
           fp);

    fclose(fp);
}

// ================= FIND ACCOUNT =================
Customer* searchAccount(int accNo)
{
    Customer *temp = head;

    while (temp != NULL)
    {
        if (temp->accountNumber == accNo)
        {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}

// ================= SYSTEM LOGIN =================
int verifyBankSystem()
{
    char pass[30];

    printf("Enter system password : ");

    scanf("%s", pass);

    if (strcmp(pass, BANK_PASSWORD) == 0)
    {
        return 1;
    }

    return 0;
}

// ================= ADMIN LOGIN =================
int verifyAdmin()
{
    char pass[30];

    printf("Enter admin password : ");

    scanf("%s", pass);

    if (strcmp(pass, ADMIN_PASSWORD) == 0)
    {
        return 1;
    }

    printf("Invalid admin password\n");

    return 0;
}

// ================= VERIFY PIN =================
int checkPin(Customer *c)
{
    int pin;

    int attempts = 0;

    if (c->accountStatus == 0)
    {
        printf("Account is locked\n");
        return 0;
    }

    while (attempts < 3)
    {
        printf("Enter ATM PIN : ");

        scanf("%d", &pin);

        if (pin == c->atmPin)
        {
            return 1;
        }

        printf("Incorrect PIN\n");

        attempts++;
    }

    c->accountStatus = 0;

    printf("Account locked due to wrong PIN attempts\n");

    return 0;
}

// ================= CREATE ACCOUNT =================
void createAccount()
{
    Customer *newCustomer;

    newCustomer =
    (Customer*) malloc(sizeof(Customer));

    printf("Enter account number : ");

    scanf("%d",
          &newCustomer->accountNumber);

    if (searchAccount(newCustomer->accountNumber))
    {
        printf("Account already exists\n");

        free(newCustomer);

        return;
    }

    printf("Enter customer name : ");

    scanf("%s",
          newCustomer->name);

    printf("Enter account category : ");

    scanf("%s",
          newCustomer->accountCategory);

    printf("Enter initial deposit : ");

    scanf("%lf",
          &newCustomer->balance);

    printf("Create ATM PIN : ");

    scanf("%d",
          &newCustomer->atmPin);

    newCustomer->accountStatus = 1;

    newCustomer->todayWithdraw = 0;

    getTodayDate(newCustomer->withdrawDate);

    getDateTime(newCustomer->createdOn);

    createCardNumber(newCustomer->debitCard);

    newCustomer->next = NULL;

    if (head == NULL)
    {
        head = newCustomer;
    }
    else
    {
        Customer *temp = head;

        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newCustomer;
    }

    saveTransaction(newCustomer->accountNumber,
                    "ACCOUNT OPENED",
                    newCustomer->balance);

    printf("\nAccount created successfully\n");

    printf("Generated Debit Card : %s\n",
           newCustomer->debitCard);
}

// ================= DEPOSIT =================
void depositMoney()
{
    int accNo;

    double amount;

    printf("Enter account number : ");

    scanf("%d", &accNo);

    Customer *c = searchAccount(accNo);

    if (c == NULL)
    {
        printf("Account not found\n");
        return;
    }

    if (!checkPin(c))
    {
        return;
    }

    printf("Enter deposit amount : ");

    scanf("%lf", &amount);

    if (amount <= 0)
    {
        printf("Invalid amount\n");
        return;
    }

    c->balance += amount;

    saveTransaction(accNo,
                    "AMOUNT DEPOSITED",
                    amount);

    printf("Deposit completed successfully\n");

    printf("Updated Balance : %.2lf\n",
           c->balance);
}

// ================= WITHDRAW =================
void withdrawMoney()
{
    int accNo;

    double amount;

    char currentDate[20];

    getTodayDate(currentDate);

    printf("Enter account number : ");

    scanf("%d", &accNo);

    Customer *c = searchAccount(accNo);

    if (c == NULL)
    {
        printf("Account not found\n");
        return;
    }

    if (!checkPin(c))
    {
        return;
    }

    if (strcmp(c->withdrawDate,
               currentDate) != 0)
    {
        c->todayWithdraw = 0;

        strcpy(c->withdrawDate,
               currentDate);
    }

    printf("Enter withdrawal amount : ");

    scanf("%lf", &amount);

    if (amount > DAILY_WITHDRAW_LIMIT)
    {
        printf("Daily limit exceeded\n");
        return;
    }

    if (amount > c->balance)
    {
        printf("Insufficient balance\n");
        return;
    }

    c->balance -= amount;

    c->todayWithdraw += amount;

    saveTransaction(accNo,
                    "CASH WITHDRAWN",
                    amount);

    printf("Please collect your cash\n");

    printf("Remaining Balance : %.2lf\n",
           c->balance);
}

// ================= BALANCE ENQUIRY =================
void balanceEnquiry()
{
    int accNo;

    printf("Enter account number : ");

    scanf("%d", &accNo);

    Customer *c = searchAccount(accNo);

    if (c == NULL)
    {
        printf("Account not available\n");
        return;
    }

    if (!checkPin(c))
    {
        return;
    }

    printf("\n========== ACCOUNT DETAILS ==========\n");

    printf("Account Number : %d\n",
           c->accountNumber);

    printf("Customer Name  : %s\n",
           c->name);

    printf("Account Type   : %s\n",
           c->accountCategory);

    printf("Current Balance: %.2lf\n",
           c->balance);

    printf("Debit Card No  : %s\n",
           c->debitCard);

    printf("Created Date   : %s\n",
           c->createdOn);

    if (c->accountStatus == 1)
    {
        printf("Account Status : ACTIVE\n");
    }
    else
    {
        printf("Account Status : LOCKED\n");
    }
}

// ================= MINI STATEMENT =================
void miniStatement()
{
    int accNo;

    printf("Enter account number : ");

    scanf("%d", &accNo);

    FILE *fp;

    fp = fopen("transactionData.dat", "rb");

    if (fp == NULL)
    {
        printf("No transaction records found\n");
        return;
    }

    Transaction t;

    printf("\n========== MINI STATEMENT ==========\n");

    while (fread(&t,
                 sizeof(Transaction),
                 1,
                 fp))
    {
        if (t.accNo == accNo)
        {
            printf("%s | %s | %.2lf\n",
                   t.dateTime,
                   t.transactionType,
                   t.amount);
        }
    }

    fclose(fp);
}

// ================= VIEW ALL ACCOUNTS =================
void displayAllAccounts()
{
    if (!verifyAdmin())
    {
        return;
    }

    Customer *temp = head;

    printf("\n========== CUSTOMER LIST ==========\n");

    while (temp != NULL)
    {
        printf("A/C NO : %d\n",
               temp->accountNumber);

        printf("NAME   : %s\n",
               temp->name);

        printf("TYPE   : %s\n",
               temp->accountCategory);

        printf("BALANCE: %.2lf\n",
               temp->balance);

        printf("-----------------------------------\n");

        temp = temp->next;
    }
}

// ================= UNLOCK ACCOUNT =================
void unlockAccount()
{
    int accNo;

    if (!verifyAdmin())
    {
        return;
    }

    printf("Enter account number : ");

    scanf("%d", &accNo);

    Customer *c = searchAccount(accNo);

    if (c == NULL)
    {
        printf("Account not found\n");
        return;
    }

    c->accountStatus = 1;

    printf("Account unlocked successfully\n");
}

// ================= MAIN MENU =================
void menu()
{
    printf("\n=====================================\n");

    printf("        DIGITAL ATM SYSTEM\n");

    printf("=====================================\n");

    printf("1. Create Account\n");
    printf("2. Deposit Money\n");
    printf("3. Withdraw Money\n");
    printf("4. Balance Enquiry\n");
    printf("5. Mini Statement\n");
    printf("6. View All Accounts\n");
    printf("7. Unlock Account\n");
    printf("8. Exit\n");

    printf("=====================================\n");

    printf("Enter your choice : ");
}

// ================= MAIN FUNCTION =================
int main()
{
    srand(time(NULL));

    if (!verifyBankSystem())
    {
        printf("Access Denied\n");
        return 0;
    }

    int choice;

    while (1)
    {
        menu();

        scanf("%d", &choice);

        switch(choice)
        {
            case 1:
                createAccount();
                break;

            case 2:
                depositMoney();
                break;

            case 3:
                withdrawMoney();
                break;

            case 4:
                balanceEnquiry();
                break;

            case 5:
                miniStatement();
                break;

            case 6:
                displayAllAccounts();
                break;

            case 7:
                unlockAccount();
                break;

            case 8:
                printf("Thank you for using ATM System\n");
                return 0;

            default:
                printf("Invalid menu option\n");
        }
    }

    return 0;
}
