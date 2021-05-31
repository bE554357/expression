#ifndef EXPRESSION_H_INCLUDED
#define EXPRESSION_H_INCLUDED
namespace my::expression
{

template <typename T>
struct is_expression : std::false_type {};

template <typename T>
inline const bool is_expression_v = is_expression<T>::value;

template <typename T>
concept expression_type = is_expression_v<T>;

template <typename T>
struct constantZero;


template <typename type>
struct constant
{
	using type_t = type;
	using result_t = type_t;

	type value;
	constexpr type getValue() const
	{
		return value;
	}

	template <typename ... args_t>
	constexpr auto setTo(args_t ...) const
	{
		return constant<type> {value};
	}

	template <typename ... vars_t>
	static constexpr auto diff(vars_t ...)
	{
		return constantZero<type_t>::zero();
	}

	std::string getString() const
	{
		return std::to_string(value);
	}

	template <typename T>
	static consteval bool isDependOn(T)
	{
		return false;
	}
};

template <typename T>
struct constantZero
{
	static constexpr auto zero()
	{
		return constant<T> {0};
	}
};

template <>
struct constantZero<matrix>
{
	static auto zero()
	{
		return constant<matrix> {matrix::Zero()};
	}
};

template <typename type, char ix>
struct settedVar
{
	using type_t = type;
	static const char ix_v = ix;
	type value;
};

template <typename type, char ix>
struct variable
{
	using type_t = type;
	using result_t = type_t;
	static const char ix_v = ix;

	constexpr settedVar<type_t, ix_v> operator()(const type& val) const
	{
		return settedVar<type_t, ix_v> {val};
	}

	template <typename T, char IX, typename ... over>
	constexpr auto setTo(const settedVar<T, IX>& var, const over& ... overVars) const
	{
		if constexpr(IX == ix_v)
			return constant<type> {var.value};
		else
			return setTo(overVars...);
	}

	template<typename T, char IX>
	constexpr auto setTo(const settedVar<T, IX>& var) const
	{
		if constexpr(IX == ix_v)
			return constant<type> {var.value};
		else
			return *this;
	}

	template <typename T1, typename T2>
	static constexpr auto diff(T1 var, T2 dVar)
	{
		static_assert(dVar.ix_v != ix_v, "dVar have to be not presented in expression");

		if constexpr(var.ix_v == ix_v)
		{
			static_assert(std::is_same<typename T1::type_t, type_t>::value, "var.type_t and this::type_t have to be the same");
			return dVar;
		}
		else
			return constantZero<type_t>::zero();
	}

	static std::string getString()
	{
		char res[2] = {ix, '\0'};
		return std::string(res);
	}

	template <typename typeIn, char N>
	static consteval bool isDependOn(variable<typeIn, N>)
	{
		//variable chech have to be agreed: only by a char or by char and type
		//if(std::is_same_v<typeIn, type_t> && N == ix_v)
		if(N == ix_v)
			return true;
		else
			return false;
	}
};


template <typename T, char Ix>
struct is_expression<variable<T, Ix>> : std::true_type {};

template <typename T>
struct is_expression<constant<T>> : std::true_type {};


}

#endif
