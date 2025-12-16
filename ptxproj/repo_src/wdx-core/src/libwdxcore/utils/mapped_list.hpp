//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-core.
//
// Copyright (c) 2024-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
//------------------------------------------------------------------------------
#ifndef SRC_LIBWDXCORE_UTILS_MAPPED_LIST_HPP_
#define SRC_LIBWDXCORE_UTILS_MAPPED_LIST_HPP_

#include <memory>

namespace wago {
namespace wdx {

// FIXME: Remove this functionality

template <typename T>
class list_i
{
public:
    // Iterator traits - modern C++17 approach instead of inheriting from std::iterator
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    int pos = 0;

    list_i()
    {
    }

    list_i(std::shared_ptr<list_i<T>> iter_)
        : iter(iter_)
    {
    }

    virtual ~list_i() noexcept = default;

    virtual list_i<T> begin() // should not be called
    {
        return list_i();
    };

    virtual list_i<T> end() // should not be called
    {
        return list_i();
    };

    virtual list_i<T>& operator++()
    {
        return iter->operator++();
    }

    virtual bool operator!=(const list_i<T>& other)
    {
        return iter->pos != other.iter->pos;
    }

    virtual T operator*() const
    {
        return **iter;
    }

    virtual T operator[](size_t idx) const
    {
        return (*iter)[idx];
    }

    virtual size_t size()
    {
        return iter->size();
    }

private:
    std::shared_ptr<list_i<T>> iter;
};

template <typename T, typename TDest>
class mapped_list
{
    const std::vector<T>* data;
    TDest (*f)(const T& element);

public:
    mapped_list<T, TDest>(const std::vector<T>& data_, TDest (*f_)(const T& element))
        : data(&data_), f(f_)
    {
    }

    //MappedList<T, TDest>(const shared_ptr<vector<T>> data, TDest(*f)(T& element)) : data(data), f(f) {}

    class mapped_list_iterator : public list_i<TDest>
    {
        const int ENDPOS = -1;
        const mapped_list* list;

    public:
        mapped_list_iterator(const mapped_list* list_)
            : list(list_)
        {
        }

        mapped_list_iterator(int pos_) : list(nullptr)
        {
            this->pos = pos_;
        }

        list_i<TDest> begin() override
        {
            return list_i<TDest>(std::make_shared<mapped_list_iterator>(list));
        }

        list_i<TDest> end() override
        {
            return list_i<TDest>(std::make_shared<mapped_list_iterator>(ENDPOS));
        }

        list_i<TDest>& operator++() override
        {
            // TODO: error if calling this on ENDPOS
            if (static_cast<unsigned>(this->pos + 1) < list->data->size())
                this->pos++;
            else
                this->pos = ENDPOS;
            return *this; // TODO
        }

        bool operator!=(const list_i<TDest>& other) override
        {
            return this->pos != other.pos;
        }

        TDest operator*() const override
        {
            return list->f(list->data->at(this->pos));
        }

        TDest operator[](size_t idx) const override
        {
            return list->f(list->data->at(idx));
        }

        size_t size() override
        {
            return list->data->size();
        }
    };

    std::shared_ptr<list_i<TDest>> get_iterator()
    {
        return std::make_shared<mapped_list_iterator>(this);
    }
};
}
}
#endif // SRC_LIBWDXCORE_UTILS_MAPPED_LIST_HPP_
