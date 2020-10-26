#pragma once

namespace tego
{
    //
    // call functor at end of scope
    //
    template<typename T>
    class scope_exit
    {
    public:
        scope_exit() = delete;
        scope_exit(const scope_exit&) = delete;
        scope_exit& operator=(const scope_exit&) = delete;
        scope_exit& operator=(scope_exit&&) =  delete;

        scope_exit(scope_exit&&) = default;
        scope_exit(T&& functor)
         : functor_(new T(std::move(functor)))
        {
            static_assert(std::is_same<void, decltype(functor())>::value);
        }

        ~scope_exit()
        {
            if (functor_.get())
            {
                functor_->operator()();
            }
        }

    private:
        std::unique_ptr<T> functor_;
    };


    auto make_scope_exit(auto&& func) ->
        scope_exit<typename std::remove_reference<decltype(func)>::type>
    {
        return {std::move(func)};
    }

    //
    // constexpr strlen for compile-time null terminated C String constants
    //
    template<size_t N>
    constexpr size_t static_strlen(const char (&str)[N])
    {
        if (str[N-1] != 0) throw "C String missing null terminator";
        for(size_t i = 0; i < (N - 1); i++)
        {
            if (str[i] == 0) throw "C String has early null terminator";
        }
        return N-1;
    }

    //
    // helper class for populating out T** params into unique_ptr<T> objects
    //
    template<typename T>
    class out_unique_ptr
    {
    public:
        out_unique_ptr() = delete;
        out_unique_ptr(const out_unique_ptr&) = delete;
        out_unique_ptr(out_unique_ptr&&) = delete;
        out_unique_ptr& operator=(const out_unique_ptr&) = delete;
        out_unique_ptr& operator=(out_unique_ptr&&) = delete;

        out_unique_ptr(std::unique_ptr<T>& u) : u_(u) {}
        ~out_unique_ptr()
        {
            u_.reset(t_);
        }

        operator T**()
        {
            return &t_;
        }

    private:
        T* t_ = nullptr;
        std::unique_ptr<T>& u_;
    };

    //
    // helper function for populating out T** params
    // example:
    //
    // void give_int(int** outInt);
    // std::unique_ptr<int> pint;
    // give_int(tego::out(pint));
    // int val = *pint;
    //
    template<typename T>
    out_unique_ptr<T> out(std::unique_ptr<T>& ptr)
    {
        return {ptr};
    }
}
