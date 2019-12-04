#pragma once

// 派生クラスをコピー禁止にする。
// private継承して使う。
class NonCopyable{
protected:
    NonCopyable(){}
    ~NonCopyable(){}

private:
    NonCopyable(const NonCopyable&);
    const NonCopyable& operator=(const NonCopyable&);
};