namespace my::expression
{


template <typename T, typename OP>
struct unaryExpression
{
	T op;

	using result_t = typename OP::template result_t<T>;

	template <typename ... vars_t>
	auto setTo(const vars_t&... vars) const
	{
		using t = decltype(op.setTo(vars...));
		return unaryExpression<t, OP> {op.setTo(vars...)};
	}

	auto getValue() const
	{
		return OP::getValue(op);
	}

	template <typename T1, typename T2>
	auto diff(T1 var, T2 dVar)
	{
		if constexpr(T::isDependOn(var))
			return OP::diff(op, var, dVar);
		else
			return constantZero<result_t>::zero();
	}

	std::string getString()
	{
		return OP::getString(op);
	}

	template <typename typeIn, char N>
	static consteval bool isDependOn(variable<typeIn, N> in)
	{
		return T::isDependOn(in);
	}
};

struct transposeOp
{
	template <typename T>
	using result_t = typename T::result_t;

	template <typename T1>
	static auto getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		return static_cast<matrix>(Op.getValue().transpose().eval());
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		return unaryExpression<decltype(dOp), transposeOp> {dOp};
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return Op.getString() + "^T";
	}
};

struct devOp
{
	template <typename T>
	using result_t = typename T::result_t;

	template <typename T1>
	static auto getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		auto tmp = Op.getValue();
		return static_cast<matrix>(tmp - (1.0/3.0)*tmp.trace()*matrix::Identity());
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		return unaryExpression<decltype(dOp), devOp> {dOp};
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return Op.getString() + "^D";
	}
};

struct traceOp
{
	template <typename T>
	using result_t = double;

	template <typename T1>
	static auto getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		auto tmp = Op.getValue();
		return tmp.trace();
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		return unaryExpression<decltype(dOp), traceOp> {dOp};
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return Op.getString() + "^D";
	}
};

//note: is defined only for matrices now. Edit diff in unimodOp, when this will be extender upon double
struct inverseOp
{
	template <typename T>
	using result_t = typename T::result_t;

	template <typename T1>
	static auto getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		return Op.getValue().inverse().eval();
	}

	template <typename T, typename T1, typename T2>
	static auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		
		return  binaryExpression<T, decltype(dOp), structInverseDerHelper> {Op, dOp};
		//return constant<double> {-1.0}*inverse(Op)*dOp*inverse(Op);
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return Op.getString() + "^-1";
	}
};

struct cbrtOp
{
	template <typename T>
	using result_t = double;

	template <typename T1>
	static auto getValue(T1 Op)
	{
		static_assert(std::is_same_v<double, typename T1::result_t>, "argument have to be double");

		return std::cbrt(Op.getValue());
	}

	template <typename T, typename T1, typename T2>
	static auto diff(T Op, T1 var, T2 dVar) = delete;

	template <typename T>
	static std::string getString(T Op)
	{
		return "cbrt(" + Op.getString() + ")";
	}
};

struct unimodOp
{
	template <typename T>
	using result_t = typename T::result_t;

	template <typename T1>
	static matrix getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		auto tmp = Op.getValue();
		return tmp/std::cbrt(tmp.determinant());
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);

		return binaryExpression<T, decltype(dOp), structUnimodularDerHelper> {Op, dOp};
		//return cbrt(det(inverse(Op)))*(constant<double> {-1.0/3.0}*dot(transpose(inverse(Op)), dOp)*Op + dOp);
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return Op.getString() + "^U";
	}
};

struct detOp
{
	template <typename T>
	using result_t = double;

	template <typename T1>
	static auto getValue(const T1& Op)
	{
		static_assert(std::is_same_v<matrix, typename T1::result_t>, "argument have to be matrix Eigen::Matrix3d");

		return Op.getValue().determinant();
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		return det(Op)*dot(transpose(inverse(Op)), dOp);
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return "det(" + Op.getString() + ")";
	}
};

struct expOp
{
	template <typename T>
	using result_t = double;

	template <typename T1>
	static double getValue(const T1 Op)
	{
		static_assert(std::is_same_v<double, typename T1::result_t>, "exp. argument have to be a double");

		return std::exp(Op.getValue());
	}

	template <typename T, typename T1, typename T2>
	static constexpr auto diff(T Op, T1 var, T2 dVar)
	{
		auto dOp = Op.diff(var, dVar);
		return exp(Op)*dOp;
	}

	template <typename T>
	static std::string getString(T Op)
	{
		return "exp(" + Op.getString() + ")";
	}
};

template <typename T, typename Op>
struct is_expression<unaryExpression<T, Op>> : std::true_type {};


template <expression_type T>
auto transpose(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "transposion is defined only for a matrices");
	return unaryExpression<T, transposeOp> {Op};
}

template <expression_type T>
auto dev(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "transposion is defined only for a matrices");
	return unaryExpression<T, devOp> {Op};
}

template <expression_type T>
auto trace(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "transposion is defined only for a matrices");
	return unaryExpression<T, traceOp> {Op};
}

template <expression_type T>
auto inverse(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "inverion YET is defined only for a matrices");
	return unaryExpression<T, inverseOp> {Op};
}

template <expression_type T>
auto cbrt(T Op)
{
	static_assert(std::is_same_v<double, typename T::result_t>, "root is defined only for a double");
	return unaryExpression<T, cbrtOp> {Op};
}

template <expression_type T>
auto unimod(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "unimodular part is defined only for a matrices");
	return unaryExpression<T, unimodOp> {Op};
}

template <expression_type T>
auto det(T Op)
{
	static_assert(std::is_same_v<matrix, typename T::result_t>, "determinant is defined only for a matrices");
	return unaryExpression<T, detOp> {Op};
}

template <expression_type T>
auto exp(T Op)
{
	static_assert(std::is_same_v<double, typename T::result_t>, "exp is defined only for a matrices");
	return unaryExpression<T, expOp> {Op};
}



}
