#pragma once
#include <string>

template<typename T>
class IValidator {
public:
    virtual ~IValidator() = default;
    virtual bool validate(const T& value) const = 0;
};

template<typename T>
class DefaultValidator : public IValidator<T> {
public:
    bool validate(const T& value) const override { return true; }
}; 