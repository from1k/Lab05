#include <gtest/gtest.h>
#include <stdexcept>
#include "Account.h"

TEST(AccountTest, ConstructorInitializesCorrectly) {
    Account acc(1, 1000);
    EXPECT_EQ(acc.id(), 1);
    EXPECT_EQ(acc.GetBalance(), 1000);
}

TEST(AccountTest, GetBalanceReturnsCorrectBalance) {
    Account acc(1, 1000);
    EXPECT_EQ(acc.GetBalance(), 1000);
}

TEST(AccountTest, ChangeBalanceThrowsWhenNotLocked) {
    Account acc(1, 1000);
    EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
}

TEST(AccountTest, ChangeBalanceWorksWhenLocked) {
    Account acc(1, 1000);
    acc.Lock();
    acc.ChangeBalance(100);
    EXPECT_EQ(acc.GetBalance(), 1100);
}

TEST(AccountTest, ChangeBalanceWorksWithNegativeDiff) {
    Account acc(1, 1000);
    acc.Lock();
    acc.ChangeBalance(-300);
    EXPECT_EQ(acc.GetBalance(), 700);
}

TEST(AccountTest, LockThrowsWhenAlreadyLocked) {
    Account acc(1, 1000);
    acc.Lock();
    EXPECT_THROW(acc.Lock(), std::runtime_error);
}

TEST(AccountTest, UnlockWorksCorrectly) {
    Account acc(1, 1000);
    acc.Lock();
    acc.ChangeBalance(100);
    acc.Unlock();
    EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
}

TEST(AccountTest, DestructorWorks) {
    Account* acc = new Account(99, 1000);
    delete acc;
}
