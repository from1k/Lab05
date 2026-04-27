#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include <sstream>
#include "Transaction.h"
#include "mock_account.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Exactly;
using ::testing::AnyNumber;

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        fromAccount = new MockAccount(1, 1000);
        toAccount = new MockAccount(2, 500);
        transaction.set_fee(10);
    }
    
    void TearDown() override {
        delete fromAccount;
        delete toAccount;
    }
    
    MockAccount* fromAccount;
    MockAccount* toAccount;
    Transaction transaction;
};

TEST_F(TransactionTest, MakeSuccessfulTransaction) {
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(150)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(1000))
        .WillOnce(Return(840));
    EXPECT_CALL(*fromAccount, ChangeBalance(-160)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(650));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 150);
    EXPECT_TRUE(result);
}

TEST_F(TransactionTest, MakeTransactionInsufficientFunds) {
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(200)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(100))
        .WillOnce(Return(100));
    EXPECT_CALL(*fromAccount, ChangeBalance(-210)).Times(0);
    EXPECT_CALL(*toAccount, ChangeBalance(-200)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(500));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 200);
    EXPECT_FALSE(result);
}

TEST_F(TransactionTest, MakeTransactionSameAccountThrows) {
    EXPECT_THROW(transaction.Make(*fromAccount, *fromAccount, 100), std::logic_error);
}

TEST_F(TransactionTest, MakeTransactionNegativeSumThrows) {
    EXPECT_THROW(transaction.Make(*fromAccount, *toAccount, -50), std::invalid_argument);
}

TEST_F(TransactionTest, MakeTransactionSumTooSmallThrows) {
    EXPECT_THROW(transaction.Make(*fromAccount, *toAccount, 50), std::logic_error);
}

TEST_F(TransactionTest, MakeTransactionFeeTooHigh) {
    transaction.set_fee(100);
    EXPECT_CALL(*fromAccount, Lock()).Times(0);
    EXPECT_CALL(*toAccount, Lock()).Times(0);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 150);
    EXPECT_FALSE(result);
}

TEST_F(TransactionTest, MakeTransactionBoundarySum) {
    transaction.set_fee(10);
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(100)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(1000))
        .WillOnce(Return(890));
    EXPECT_CALL(*fromAccount, ChangeBalance(-110)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(600));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 100);
    EXPECT_TRUE(result);
}

TEST_F(TransactionTest, MakeTransactionSumLessThanDoubleFee) {
    transaction.set_fee(60);
    bool result = transaction.Make(*fromAccount, *toAccount, 100);
    EXPECT_FALSE(result);
}

TEST_F(TransactionTest, MakeTransactionWithDifferentFees) {
    transaction.set_fee(5);
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(200)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(1000))
        .WillOnce(Return(795));
    EXPECT_CALL(*fromAccount, ChangeBalance(-205)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(700));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 200);
    EXPECT_TRUE(result);
    EXPECT_EQ(transaction.fee(), 5);
}

TEST_F(TransactionTest, MakeTransactionExactBalance) {
    delete fromAccount;
    fromAccount = new MockAccount(1, 210);
    
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(200)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(210))
        .WillOnce(Return(0));
    EXPECT_CALL(*fromAccount, ChangeBalance(-210)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(700));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 200);
    EXPECT_TRUE(result);
}

TEST_F(TransactionTest, MakeTransactionRollbackOnFailure) {
    EXPECT_CALL(*fromAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, Lock()).Times(1);
    EXPECT_CALL(*toAccount, ChangeBalance(300)).Times(1);
    EXPECT_CALL(*fromAccount, GetBalance())
        .WillOnce(Return(200))
        .WillOnce(Return(200));
    EXPECT_CALL(*fromAccount, ChangeBalance(-310)).Times(0);
    EXPECT_CALL(*toAccount, ChangeBalance(-300)).Times(1);
    EXPECT_CALL(*toAccount, GetBalance()).WillOnce(Return(500));
    EXPECT_CALL(*fromAccount, Unlock()).Times(1);
    EXPECT_CALL(*toAccount, Unlock()).Times(1);
    
    bool result = transaction.Make(*fromAccount, *toAccount, 300);
    EXPECT_FALSE(result);
}

TEST_F(TransactionTest, MakeTransactionLockThrows) {
    EXPECT_CALL(*fromAccount, Lock()).WillOnce(::testing::Throw(std::runtime_error("already locked")));
    EXPECT_CALL(*toAccount, Lock()).Times(0);
    
    EXPECT_THROW(transaction.Make(*fromAccount, *toAccount, 200), std::runtime_error);
}

class TransactionRealAccountTest : public ::testing::Test {
protected:
    void SetUp() override {
        from = new Account(1, 1000);
        to = new Account(2, 500);
        transaction.set_fee(10);
    }
    
    void TearDown() override {
        delete from;
        delete to;
    }
    
    Account* from;
    Account* to;
    Transaction transaction;
};

TEST_F(TransactionRealAccountTest, MakeSuccessfulTransaction) {
    bool result = transaction.Make(*from, *to, 150);
    EXPECT_TRUE(result);
    EXPECT_EQ(from->GetBalance(), 840);
    EXPECT_EQ(to->GetBalance(), 650);
}

TEST_F(TransactionRealAccountTest, MakeTransactionInsufficientFunds) {
    Account poor(3, 100);
    bool result = transaction.Make(poor, *to, 150);
    EXPECT_FALSE(result);
    EXPECT_EQ(poor.GetBalance(), 100);
    EXPECT_EQ(to->GetBalance(), 500);
}

TEST_F(TransactionRealAccountTest, MakeTransactionBoundarySum) {
    bool result = transaction.Make(*from, *to, 100);
    EXPECT_TRUE(result);
    EXPECT_EQ(from->GetBalance(), 890);
    EXPECT_EQ(to->GetBalance(), 600);
}

TEST_F(TransactionRealAccountTest, MakeTransactionWithDifferentFees) {
    transaction.set_fee(5);
    bool result = transaction.Make(*from, *to, 200);
    EXPECT_TRUE(result);
    EXPECT_EQ(from->GetBalance(), 795);
    EXPECT_EQ(to->GetBalance(), 700);
}

TEST_F(TransactionRealAccountTest, MakeTransactionExactBalance) {
    Account exact(3, 210);
    bool result = transaction.Make(exact, *to, 200);
    EXPECT_TRUE(result);
    EXPECT_EQ(exact.GetBalance(), 0);
    EXPECT_EQ(to->GetBalance(), 700);
}

TEST_F(TransactionRealAccountTest, MakeTransactionRollbackOnFailure) {
    Account poor(3, 200);
    bool result = transaction.Make(poor, *to, 300);
    EXPECT_FALSE(result);
    EXPECT_EQ(poor.GetBalance(), 200);
    EXPECT_EQ(to->GetBalance(), 500);
}
