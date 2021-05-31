namespace my::expression
{


template <typename L, typename R, typename OP>
struct binaryExpression
{
	L lOp;
	R rOp;

	using result_t = typename OP::template result_t<L, R>;

	template <typename ... vars_t>
	auto setTo(const vars_t&... vars) const
	{
		using lOp_t = decltype(lOp.setTo(vars...));
		using rOp_t = decltype(rOp.setTo(vars...));

		return binaryExpression<lOp_t, rOp_t, OP> {lOp.setTo(vars...), rOp.setTo(vars...)};
	}

	auto getValue() const
	{
		return OP::getValue(lOp, rOp);
	}

	template <typename T1, typename T2>
	constexpr auto diff(T1 var, T2 dVar) const
	{
		return OP::diff(lOp, rOp, var, dVar);
	}

	std::string getString() const
	{
		return OP::getString(lOp, rOp);
	}

	template <typename typeIn, char N>
	static consteval bool isDependOn(variable<typeIn, N> in)
	{
		return L::isDependOn(in) || R::isDependOn(in);
	}
};


struct addOp
{
	template <typename L, typename R>
	using result_t = typename L::result_t;

	template <typename T1, typename T2>
	static auto getValue(const T1& lOp, const T2& rOp)
	{
		static_assert(std::is_same_v<typename T1::result_t, typename T2::result_t>, "addition is defined only for the same types");

		if constexpr(std::is_same_v<result_t<T1, T2>, matrix>)
			return (lOp.getValue() + rOp.getValue()).eval();
		else
			return (lOp.getValue() + rOp.getValue());
	}

	template <typename L, typename R, typename T1, typename T2>
	constexpr static auto diff(L lOp, R rOp, T1 var, T2 dVar)
	{
		auto lNew = lOp.diff(var, dVar);
		auto rNew = rOp.diff(var, dVar);

		if constexpr(lOp.isDependOn(var) && rOp.isDependOn(var))
			return binaryExpression<decltype(lNew), decltype(rNew), addOp> {lNew, rNew};
		else if constexpr(lOp.isDependOn(var) && !rOp.isDependOn(var))
			return lNew;
		else if constexpr(!lOp.isDependOn(var) && rOp.isDependOn(var))
			return rNew;
		else if constexpr(!lOp.isDependOn(var) && !rOp.isDependOn(var))
			return constantZero<result_t<L, R>>::zero();
	}

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "(" + lOp.getString() + "+" + rOp.getString() + ")";
	}
};


struct substractionOp
{
	template <typename L, typename R>
	using result_t = typename L::result_t;

	template <typename T1, typename T2>
	static auto getValue(const T1& lOp, const T2& rOp)
	{
		static_assert(std::is_same_v<typename T1::result_t, typename T2::result_t>, "sustraction is defined only for the same types");

		if constexpr(std::is_same_v<result_t<T1, T2>, matrix>)
			return (lOp.getValue() - rOp.getValue()).eval();
		else
			return (lOp.getValue() - rOp.getValue());
	}

	template <typename L, typename R, typename T1, typename T2>
	constexpr static auto diff(L lOp, R rOp, T1 var, T2 dVar)
	{
		auto lNew = lOp.diff(var, dVar);
		auto rNew = rOp.diff(var, dVar);

		constant<double> _1{-1.0};

		if constexpr(lOp.isDependOn(var) && rOp.isDependOn(var))
			return binaryExpression<decltype(lNew), decltype(rNew), substractionOp> {lNew, rNew};
		else if constexpr(lOp.isDependOn(var) && !rOp.isDependOn(var))
			return lNew;
		else if constexpr(!lOp.isDependOn(var) && rOp.isDependOn(var))
			return _1*rNew;
		else if constexpr(!lOp.isDependOn(var) && !rOp.isDependOn(var))
			return constantZero<result_t<L, R>>::zero();
	}

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "(" + lOp.getString() + "-" + rOp.getString() + ")";
	}
};

struct multOp
{
	template <typename L, typename R>
	using result_t = std::conditional_t<std::is_same_v<matrix, typename L::result_t> || std::is_same_v<matrix, typename R::result_t>, matrix, double>;

	template <typename T1, typename T2>
	static auto getValue(const T1& lOp, const T2& rOp)
	{
		if constexpr(std::is_same_v<result_t<T1, T2>, matrix>)
			return (lOp.getValue()*rOp.getValue()).eval();
		else
			return lOp.getValue()*rOp.getValue();
	}

	template <typename L, typename R, typename T1, typename T2>
	static auto diff(L lOp, R rOp, T1 var, T2 dVar)
	{
		auto dLop = lOp.diff(var, dVar);
		auto dRop = rOp.diff(var, dVar);

		if constexpr(lOp.isDependOn(var) && rOp.isDependOn(var))
			return dLop*rOp + lOp*dRop;
		else if constexpr(lOp.isDependOn(var) && !rOp.isDependOn(var))
			return dLop*rOp;
		else if constexpr(!lOp.isDependOn(var) && rOp.isDependOn(var))
			return lOp*dRop;
		else if constexpr(!lOp.isDependOn(var) && !rOp.isDependOn(var))
			return constantZero<result_t<L, R>>::zero();
	}

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "("+ lOp.getString() + "*"+rOp.getString() + ")";
	}
};

struct dotOp
{
	template <typename L, typename R>
	using result_t = double;

	template <typename T1, typename T2>
	static auto getValue(const T1& lOp, const T2& rOp)
	{
		static_assert(std::is_same_v<typename T1::result_t, matrix> && std::is_same_v<typename T2::result_t, matrix>, "dot is defined only for a matrices");

		return (lOp.getValue().array()*rOp.getValue().array()).sum();
	}

	template <typename L, typename R, typename T1, typename T2>
	static auto diff(L lOp, R rOp, T1 var, T2 dVar) = delete;

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "("+ lOp.getString() + ":"+rOp.getString() + ")";
	}
};

/* This class is for perfomance optimisation of derivative unimodular part of the tensor
 * d(uni(X)) = det(X)^(-1)*(-1/3*(X^(-T):dX)X + dX)
 * In base class (binaryExpression), lOp stands for X, rOp stands for dX
 */
struct structUnimodularDerHelper
{
	template <typename L, typename R>
	using result_t = matrix;

	template <typename T1, typename T2>
	static matrix getValue(const T1& lOp, const T2& rOp)
	{
		static_assert(std::is_same_v<typename T1::result_t, matrix> && std::is_same_v<typename T2::result_t, matrix>, "dot is defined only for a matrices");

		matrix X = lOp.getValue();
		matrix dX = rOp.getValue();

		return 1.0/std::cbrt(X.determinant())*(-1.0/3.0*((X.inverse().transpose().array()*dX.array()).sum())*X+dX);
	}

	template <typename L, typename R, typename T1, typename T2>
	static auto diff(L lOp, R rOp, T1 var, T2 dVar) = delete;

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "(det(" + lOp.getString() +")^(-1)*((-1/3*("+lOp.getString()+"^-T :" + rOp.getString() +")*"+lOp.getString()+")"+rOp.getString()+ ")";
	}
};

/* This class is for perfomance optimisation of derivative inverse of the tensor
 * d(X^-1) = -X^-1*dX*X^-1
 * In base class (binaryExpression), lOp stands for X, rOp stands for dX
 */
struct structInverseDerHelper
{
	template <typename L, typename R>
	using result_t = matrix;

	template <typename T1, typename T2>
	static matrix getValue(const T1& lOp, const T2& rOp)
	{
		static_assert(std::is_same_v<typename T1::result_t, matrix> && std::is_same_v<typename T2::result_t, matrix>, "dot is defined only for a matrices");

		matrix XInv = lOp.getValue().inverse();
		matrix dX = rOp.getValue();

		return -XInv*dX*XInv;
	}

	template <typename L, typename R, typename T1, typename T2>
	static auto diff(L lOp, R rOp, T1 var, T2 dVar) = delete;

	template <typename T1, typename T2>
	static std::string getString(T1 lOp, T2 rOp)
	{
		return "(-"+lOp.getString()+ "^(-1)*"+rOp.getString() + "*" + lOp.getString() + "^(-1))";
	}
};




template <typename T1, typename T2, typename Op>
struct is_expression<binaryExpression<T1, T2, Op>> : std::true_type {};


template <expression_type T1, expression_type T2>
auto dot(T1 lOp, T2 rOp)
{
	static_assert(std::is_same_v<typename T1::result_t, matrix> && std::is_same_v<typename T2::result_t, matrix>, "dot is defined only for a matrices");
	return binaryExpression<T1, T2, dotOp> {lOp, rOp};
}

template <expression_type T1, expression_type T2>
auto operator*(T1 lOp, T2 rOp)
{
	return binaryExpression<T1, T2, multOp> {lOp, rOp};
}

template <expression_type T1, expression_type T2>
auto operator+(T1 lOp, T2 rOp)
{
	static_assert(std::is_same_v<typename T1::result_t, typename T2::result_t>, "addition is defined only for the same types");
	return binaryExpression<T1, T2, addOp> {lOp, rOp};
}

template <expression_type T1, expression_type T2>
auto operator-(T1 lOp, T2 rOp)
{
	static_assert(std::is_same_v<typename T1::result_t, typename T2::result_t>, "sustraction is defined only for the same types");
	return binaryExpression<T1, T2, substractionOp> {lOp, rOp};
}

}
