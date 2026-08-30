#include <iostream>
#include <functional>
#include <iterator>
#include <map>
#include <set>

#include "isohedral.h"
#include "words.h"

using namespace std;

using Factor = std::pair<uint16_t, uint16_t>;
using FactorPair = std::pair<Factor, Factor>;

/*
static const xform<int8_t> square_rots[4] = {
	{1, 0, 0, 0, 1, 0},    // rotate 0
	{0, -1, 0, 1, 0, 0},   // rotate 90
	{-1, 0, 0, 0, -1, 0},  // rotate 180
	{0, 1, 0, -1, 0, 0}};  // rotate 270
*/

static const xform<int8_t> square_refls[4] = {
	{0, -1, 0, -1, 0, 0},  // reflect in line at -45 degrees
	{1, 0, 0, 0, -1, 0},   // reflect 0 
	{0, 1, 0, 1, 0, 0},	   // reflect 45
	{-1, 0, 0, 0, 1, 0}};  // reflect 90

/*
static const xform<int8_t> hex_rots[6] = {
	{1, 0, 0, 0, 1, 0},
	{0, -1, 0, 1, 1, 0},
	{-1, -1, 0, 1, 0, 0},
	{-1, 0, 0, 0, -1, 0},
	{0, 1, 0, -1, -1, 0},
	{1, 1, 0, -1, 0, 0}};
*/

static const xform<int8_t> hex_refls[6] = {
	{0, 1, 0, 1, 0, 0},   // reflect 30
	{-1, 0, 0, 1, 1, 0},  // reflect 60
	{-1, -1, 0, 0, 1, 0}, // reflect 90
	{0, -1, 0, -1, 0, 0}, // reflect 120
	{1, 0, 0, -1, -1, 0}, // reflect 150
	{1, 1, 0, 0, -1, 0}}; // reflect 180

static vector<Factor> admissible_mirror_factors(const word auto& w)
{
	size_t n = w.size();
	// TODO: make this an out parameter instead of a return value.
	// Then store it as a member field of IsohedralChecker.
	vector<Factor> factors;
	
	// Compute admissible mirror factors starting between letter pairs
	for (size_t i = 0; i < n; ++i) {
		size_t l = longestMatch(
			~(w + slice(w, 0, i)), slice(w, (i + n / 2) % n) + w, n / 4);
		size_t r = longestMatch(
			slice(w, i) + w, ~(w + slice(w, 0, (i + n / 2) % n)), n / 4);

		if (l == r && r > 0) {
		  size_t start = (i - l + n) % n;
		  size_t end = (i - 1 + r + n) % n;
		  factors.emplace_back(start, end);
		}
	}

	// Compute admissible mirror factors starting in middle of a letter
	for (size_t i = 0; i < n; ++i) {
		if (w[i] == -w[(i + n/2) % n]) {
			size_t l = longestMatch(
				~(w + slice(w, 0, i)), slice(w, (i + n / 2 + 1) % n) + w, 
				(n - 2) / 4);
			size_t r = longestMatch(
				slice(w, (i + 1) % n) + w, ~(w + slice(w, 0, (i + n / 2) % n)),
				(n - 2) / 4);

			if (l == r) {
				size_t start = (i - l + n) %n;
				size_t end = (i + r) % n;
				factors.emplace_back(start, end);
			}
		}
	}

	return factors;
}

static bool has_translation_tiling(const word auto& w, 
	const vector<Factor>& mirror_factors)
{
	// Looking for factorization A B C A_hat B_hat C_hat 
	// where A_hat is inverse complement of A.  [Case 4, IH01]
	size_t n = w.size();
	vector<set<Factor>> factor_starts(n);
	vector<set<Factor>> factor_ends(n);

	for (const auto& f: mirror_factors) {
		factor_starts[f.first].insert(f);
		factor_ends[f.second].insert(f);
	}

	for (int i = 0; i < n; ++i) {
		for (auto& A: factor_starts[i]) {
			for (auto& B: factor_starts[(A.second + 1) %n]) {
				int AB_len = A.second - A.first + 1 + n * (A.second < A.first)
					+ B.second - B.first + 1 + n * (B.second < B.first);
				if (AB_len > n / 2) {
					continue;
				}
				// C is empty.
				if (AB_len == n / 2) {
					return true;
				}
				Factor C = {(B.second + 1)%n, (A.first + n/2 - 1 + n)%n};
				if (factor_starts[C.first].find(C) 
						!= factor_starts[C.first].end()) {
					return true;
				}
			}
		}
	}

	return false;
}

static vector<Factor> admissible_rotadrome_factors(const word auto& w, 
	const xform<int8_t>& T, bool pal = false)
{
	size_t n = w.size();
	vector<Factor> factors;

	for (size_t i = 0; i < n; ++i) {
	/*
		cerr << "i = " << i << endl;
		cerr << "    " << (!(w + slice(w, 0, i))) << endl;
		cerr << "    " << ((slice(w, i) + w) * T) << endl;
	*/

		int l = longestMatch(
			!(w + slice(w, 0, i)), (slice(w, i) + w) * T, n / 2);
		if (l > 0) {
			factors.emplace_back((n + i - l) % n, (i + l - 1) % n);
		}
	}

	// Compute admissible palindrome factors starting in middle of a letter
	if (pal) {
		for (size_t i = 0; i < n; ++i) {
			int l = longestMatch(
				!(w + slice(w, 0, i)), slice(w, (i + 1) % n) + w, n / 2);
			factors.emplace_back((n + i - l) % n, (i + l) % n);
		}
	}

	return factors;
}

static vector<FactorPair> admissible_gapped_mirror_factor_pairs(
	const word auto& w)
{
	size_t n = w.size();
	vector<FactorPair> factor_pairs;

	// Compute admissible mirror factors starting between letter pairs
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = i + 1; j < n; ++j) {
			size_t l = longestMatch(
				~(w + slice(w, 0, i)), slice(w, j) + w, (i + n - j) / 2);
			size_t r = longestMatch(
				slice(w, i) + w, ~(w + slice(w, 0, j)), (j - i) / 2);

			if (l == r && r > 0) {
				factor_pairs.emplace_back(
					make_pair((n + i - l) % n, (n + i - 1 + r) % n),
					make_pair((n + j - l) % n, (n + j - 1 + r) % n));
			}
		}
	}

	// Compute admissible mirror factors starting in the middle of a letter
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = i + 1; j < n; ++j) {
			if (w[i] == -w[j]) {
				size_t l = longestMatch(
					~(w + slice(w, 0, i)), slice(w, (j + 1) % n) + w, 
					(i + n - j - 1) / 2);
				size_t r = longestMatch(
					slice(w, (i + 1) % n) + w, ~(w + slice(w, 0, j)), 
					(j - i - 1) / 2);

				if (l == r) {
					factor_pairs.emplace_back(
						make_pair((n + i - l) % n, (i + r) % n),
						make_pair((n + j - l) % n, (j + r) % n));
				}
			}
		}
	}

	return factor_pairs;
}

static bool is_double_palindrome(const Factor& F, 
	const vector<vector<Factor>>& palindrome_factor_starts, 
	const vector<vector<Factor>>& palindrome_factor_ends, size_t n) 
{
	// int F_len = F.second - F.first + 1 + n * (F.second < F.first);
	int F_len = (n + F.second - F.first + 1) % n;
	for (const auto& F1: palindrome_factor_starts[F.first]) {
		size_t F1_len = (n + F1.second - F1.first + 1) % n;
		if (F1_len == F_len) {
			return true;
		}

		for (const auto& F2: palindrome_factor_ends[F.second]) {
			size_t F2_len = (n + F2.second - F2.first + 1) % n;
			if (F_len == F1_len + F2_len) {
				return true;
			}
		}
	}
	return false;
}

static bool has_half_turn_tiling(const word auto& w, 
	const vector<FactorPair>& mirror_factor_pairs, 
	const vector<Factor>& palin_factors) 
{
	/*
    cerr << "Mirror factor pairs:" << endl;
    for (const auto& p: mirror_factor_pairs) {
        cerr << "   " << p.first.first << ", " << p.first.second
            << " / " << p.second.first << ", " << p.second.second << endl;
    }
    cerr << "Palin factors:" << endl;
    for (const auto& p: palin_factors) {
        cerr << "   " << p.first << ", " << p.second << endl;
    } 
	*/

	// Looking for factorization A B C hat(A) D E
	// where B, C, D, E are palindromes [Case 1, IH04]
	size_t n = w.size();
	vector<vector<Factor>> palindrome_factor_starts(n);
	vector<vector<Factor>> palindrome_factor_ends(n);
	for (auto& f: palin_factors) {
		palindrome_factor_starts[f.first].push_back(f);
		palindrome_factor_ends[f.second].push_back(f);
	}

	for (const auto& p: mirror_factor_pairs)  {
		Factor A = p.first;
		Factor A_hat = p.second;

		Factor dpi1 = make_pair((A.second + 1) % n, (A_hat.first - 1 + n) % n);
		Factor dpi2 = make_pair((A_hat.second + 1) % n, (A.first - 1 + n) % n);
		bool dp1 = is_double_palindrome(
			dpi1, palindrome_factor_starts, palindrome_factor_ends, n);
		bool dp2 = is_double_palindrome(
			dpi2, palindrome_factor_starts, palindrome_factor_ends, n);

		if (dp1 && dp2) {
			return true;
		}
	}

	// A is empty
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			if (j == i) {
				continue;
			}
			Factor dpi1 = make_pair(i, (j - 1 + n) % n);
			Factor dpi2 = make_pair(j % n, (i - 1 + n) % n);
			bool dp1 = is_double_palindrome(
				dpi1, palindrome_factor_starts, palindrome_factor_ends, n);
			bool dp2 = is_double_palindrome(
				dpi2, palindrome_factor_starts, palindrome_factor_ends, n);

			if (dp1 && dp2) {
				return true;
			}
		}
	}

	return false;
}

static bool has_quarter_turn_tiling(const word auto& w, 
	const vector<Factor>& ninety_factors, const vector<Factor>& palin_factors)
{
/*
	cerr << "ninety_factors" << endl;
	for (const auto& f: ninety_factors) {
		cerr << "    " << f.first << ", " << f.second << endl;
	}
	cerr << "palin_factors" << endl;
	for (const auto& f: palin_factors) {
		cerr << "    " << f.first << ", " << f.second << endl;
	}
	*/

	// Looking for factorization A B C D E
	// where A is a palindrome and B,C / D,E are 90-dromes. [Case 9, IH28]
	size_t n = w.size();
	vector<vector<Factor>> ninety_factor_starts(n);
	for (const auto& f: ninety_factors) {
		ninety_factor_starts[f.first].push_back(f);
	}

	// Factorizations with non-empty A 
	for (const auto& C: palin_factors) {
		int C_len = (n + C.second - C.first + 1) % n;

		for (const auto& A: ninety_factor_starts[(C.second + 1) % n]) {
			size_t A_len = (n + A.second - A.first + 1) % n;
			// B empty (maybe impossible, according to Winslow)
			if (A_len + C_len == n) {
				return true;
			}

			// No empty 90-drome factors
			for (const auto& B: ninety_factor_starts[(A.second + 1) % n]) {
				size_t B_len = (n + B.second - B.first + 1) % n;
				if (A_len + B_len + C_len == n) {
					return true;
				}
			}
		}
	}

	// Factorizations with empty A. 
	for (const auto& A: ninety_factors) {
		for (const auto& B: ninety_factor_starts[(A.second + 1) % n]) {
			int AB_len = (n + A.second - A.first + 1) % n
				+ (n + B.second - B.first + 1) % n;
			if (AB_len == n) {
				return true;
			}
		}
	}

	return false;
}

static bool is_glide_factor(const word auto& w, size_t i, size_t j, 
	const xform<int8_t>& T)
{
	size_t n = w.size();
	size_t l = (n + j - i + 1) % n;

	if (l % 2 != 0) {
		return false;
	}
	l /= 2;

	return l == longestMatch(
		slice(w, i) + w, (slice(w, (i + l) % n) + w) * T, l + 1);
}

static vector<Factor> admissible_glide_factors(const word auto& w,
	const xform<int8_t> *Ts, size_t num) 
{
	size_t n = w.size();
	set<Factor> factors;

	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			if (j == i) {
				continue;
			}

			for (size_t tidx = 0; tidx < num; ++tidx) {
				if (is_glide_factor(w, i, j, Ts[tidx])) {
					factors.emplace(i, j);
				}
			}
		}
	}

	return vector<Factor> {factors.begin(), factors.end()};
}

static bool has_type_1_reflection_tiling(const word auto& w, 
	const vector<Factor>& glide_factors, 
	const vector<FactorPair>& mirror_factor_pairs) 
{
	// Looking for factorization A B f_theta(B) A_hat C f_phi(C)
	// for some angles theta, phi.  [Case 3, IH02]
	size_t n = w.size();
	vector<set<Factor>> glide_factor_starts(n);
	for (auto& f: glide_factors){
		glide_factor_starts[f.first].insert(f);
	}

	for (const auto& p: mirror_factor_pairs) {
		Factor A = p.first;
		Factor A_hat = p.second;
		Factor rem1 = {(A.second + 1) % n, (A_hat.first - 1 + n) % n};
		Factor rem2 = {(A_hat.second + 1) % n, (A.first - 1 + n) % n};

		if (glide_factor_starts[rem1.first].find(rem1) 
				!= glide_factor_starts[rem1.first].end() &&
			glide_factor_starts[rem2.first].find(rem2) 
				!= glide_factor_starts[rem2.first].end()) {
			return true;
		}
	}

	for (const auto& f: glide_factors) {
		size_t f_len = f.second - f.first + 1 + n * (f.second < f.first);
		for (const auto& of: glide_factor_starts[(f.second + 1) % n]) {
			size_t of_len = of.second - of.first + 1 + n * (of.second < of.first);
			if (f_len + of_len == n) {
				return true;
			}
		}
	}

	return false;
}

static vector<FactorPair> admissible_gapped_glide_factor_pairs(
	const word auto& w, const xform<int8_t>& T) 
{
	size_t n = w.size();
	vector<FactorPair> factor_pairs;

	for (size_t i = 0; i < n; ++i) {
		for (size_t j = i + 1; j < n; ++j) {
			size_t d = min(j - i + n * (j < i), i - j + n * (i < j));
			size_t l = longestMatch(slice(w, i) + w, (slice(w, j) + w) * T, d+1);
			if (1 <= l && l <= d) {
				factor_pairs.push_back({{i, (i+l-1+n)%n}, {j, (j+l-1+n)%n}});
			}
		}
	}
	return factor_pairs;
}

static bool has_type_2_reflection_tiling(const word auto& w,
	const vector<Factor>& mirror_factors, 
	const xform<int8_t> *Ts, size_t num) 
{
	// Looking for factorization A B C A_hat f_theta(C) f_theta(B)
	// [Case 5, IH03]
	size_t n = w.size();
	for (size_t idx = 0; idx < num; ++idx) {
		const xform<int8_t>& T = Ts[idx];
		map<Factor, vector<pair<Factor, Factor>>> reflect_factor_tips;
		for (auto& p: admissible_gapped_glide_factor_pairs(w, T)) {
			Factor f = p.first;
			Factor cf = p.second;

			reflect_factor_tips[make_pair(f.first, cf.second)].push_back({f, cf});
			reflect_factor_tips[make_pair(cf.first, f.second)].push_back({cf, f});
		}

		for (auto& A: mirror_factors) {
			size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
			Factor rem1 = {(A.second+1)%n, (A.first-1+n)%n};

			for (auto& p: reflect_factor_tips[rem1]) {
				Factor f1 = p.first;
				Factor cf1 = p.second;
				Factor rem2 = {(f1.second + 1) % n, (cf1.first - 1 + n) % n};
				size_t rem2_len = rem2.second - rem2.first + 1 
					+ n * (rem2.second < rem2.first);
				if (rem2_len == A_len) {
					return true;
				}

				for (auto& p2: reflect_factor_tips[rem2]) {
					Factor f2 = p2.first;
					Factor cf2 = p2.second;
					Factor rem3 = {(f2.second + 1) % n, (cf2.first - 1 + n) % n};
					size_t rem3_len = rem3.second - rem3.first + 1 
						+ n * (rem3.second < rem3.first);
					if (rem3_len == A_len) {
						return true;
					}
				}
			}
		}

		for (size_t i = 0; i < n; ++i) {
			for (auto& p: reflect_factor_tips[{(i+1)%n, i}]) {
				Factor f1 = p.first;
				Factor cf1 = p.second;
				Factor rem2 = {(f1.second + 1) % n, (cf1.first - 1 + n) % n};
				for (auto& p2: reflect_factor_tips[rem2]) {
					Factor f2 = p2.first;
					Factor cf2 = p2.second;
					if ((f2.second + 1) % n == cf2.first) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

static bool has_type_1_half_turn_reflection_tiling(const word auto& w, 
	const vector<FactorPair>& partial_mirror_factor_pairs, 
	const vector<Factor>& palin_factors, 
	const vector<Factor>& glide_factors) 
{
/*
	cerr << "Glides" << endl;
	for (const auto& f: glide_factors) {
		cerr << "    " << f.first << ", " << f.second << endl;
	}
	*/

	// Looking for factorization A B C A_Hat D f_theta(D)
	// [Case 2, IH05]
	size_t n = w.size();
	// Factorization is not symmetric so we need both orderings of each pair.
	vector<FactorPair> mirror_factor_pairs;
	for (const auto& p: partial_mirror_factor_pairs) {
		mirror_factor_pairs.push_back(p);
		mirror_factor_pairs.push_back({p.second, p.first});
	}
  
	vector<vector<Factor>> palindrome_factor_starts(n);
	vector<vector<Factor>> palindrome_factor_ends(n);
	for (const auto& f: palin_factors) {
		palindrome_factor_starts[f.first].push_back(f);
		palindrome_factor_ends[f.second].push_back(f);
	}
	vector<vector<Factor>> glide_starts(n);
	for (const auto& f: glide_factors) {
		glide_starts[f.first].push_back(f);
	}

	// Non-empty mirrors 
	for (const auto& p: mirror_factor_pairs) {
		Factor A = p.first;
		Factor A_hat = p.second;
		Factor dpi1 = {(A.second + 1) % n, (n + A_hat.first - 1) % n};
		Factor dpi2 = {(A_hat.second + 1) % n, (n + A.first - 1) % n};
		bool dp1 = is_double_palindrome(
			dpi1, palindrome_factor_starts, palindrome_factor_ends, n);
		// Other variant (dpi2 is the double palindrome) checked later 
		// by symmetry of A, A_hat
		vector<Factor>& starts = glide_starts[dpi2.first];
		if (dp1 && (find(starts.begin(), starts.end(), dpi2) != starts.end())) {
			return true;
		}
	}
  
	// Empty mirrors
	for (const auto& dp1: glide_factors) {
		bool dp2 = is_double_palindrome(
			{(dp1.second + 1) % n, (n + dp1.first - 1) % n},
			palindrome_factor_starts, palindrome_factor_ends, n);
		if (dp2) {
			return true;
		}
	}

	return false;
}

static bool has_type_2_half_turn_reflection_tiling(const word auto& w,
	const vector<Factor>& palin_factors, const xform<int8_t> *Ts, size_t num) 
{
	// Looking for factorization A B C D f_theta(B) f_phi(D)
	// where A, C are palindromes and theta - phi = +-90
	// [Case 6, IH06]
	size_t n = w.size();
	map<size_t, vector<FactorPair>> reflect_factor_pairs;

	for (size_t idx = 0; idx < num; ++idx) {
		auto gfp = admissible_gapped_glide_factor_pairs(w, Ts[idx]);
		size_t ngfp = gfp.size();

		// Double up, both for tips and for f1, cf1 iterating since B, 
		// refl(B) are not symmetric
		for (size_t jdx = 0; jdx < ngfp; ++jdx) {
			const auto& fp = gfp[jdx];
			gfp.emplace_back(fp.second, fp.first);
		}

		reflect_factor_pairs[idx] = std::move(gfp);
	}

	// TODO: Verify that this is the correct computation regardless of
	// grid rotational order.
	for (size_t theta1 = 0; theta1 < num; ++theta1) {
		size_t theta2 = (theta1 + num / 2) % num;

		map<Factor, vector<pair<Factor, Factor>>> reflect_factor_tips;
		for (auto& pf: reflect_factor_pairs[theta2]) {
			Factor f = pf.first;
			Factor cf = pf.second;
			reflect_factor_tips[{f.first, cf.second}].push_back(make_pair(f, cf));
		}

		// Non-empty B, refl(B)
		for (auto& pf: reflect_factor_pairs[theta1]) {
			Factor f1 = pf.first;
			Factor cf1 = pf.second;
			Factor dpi1 = {(f1.second+1)%n, (cf1.first-1+n)%n};
			Factor dpi2 = {(cf1.second+1)%n, (f1.first-1+n)%n};

			// Empty D, refl(D)
			if ((find(palin_factors.begin(), palin_factors.end(), dpi1) 
				!= palin_factors.end()) && 
				(find(palin_factors.begin(), palin_factors.end(), dpi2) 
				!= palin_factors.end())) {
				return true;
			}

			// Non-empty D, refl(D)
			for (auto& pr: reflect_factor_tips[{dpi1.first, dpi2.second}]) {
				Factor f2 = pr.first;
				Factor cf2 = pr.second;
				vector<Factor> rem1f;
				if ((f2.second + 1) % n == cf1.first) { // |A| = 0
					rem1f = {f2};
				}
				Factor rem1i = {(f2.second + 1) % n, dpi1.second};

				if (rem1f.empty() && 
					(find(palin_factors.begin(), palin_factors.end(), rem1i)
						!= palin_factors.end())) {
					rem1f = {f2, rem1i};
				}

				vector<Factor> rem2f;
				if (cf1.second == (cf2.first - 1 + n) % n) { // |C| = 0
					rem2f = {cf2};
				}
				Factor rem2i = {dpi2.first, (cf2.first - 1 + n) % n};
				if (rem2f.empty() && 
					(find(palin_factors.begin(), palin_factors.end(), rem2i) 
						!= palin_factors.end())) {
					rem2f = {rem2i, cf2};
				}

				if (!rem1f.empty() && !rem2f.empty()) {
					return true;
				}
			}

			// Empty B, refl(B): D refl(D) A C with A, C, palindromes
			for (size_t i = 0; i < n; ++i) {
				for (auto& pr: reflect_factor_tips[{(i+1)%n, i}]){
					Factor f2 = pr.first;
					Factor cf2 = pr.second;
					if ((f2.second + 1) % n == cf2.first && 
						(cf2.second + 1) % n == f2.first) {
						return true;
					}
					for (auto& p1: palin_factors) {
						if (p1.first != (cf2.second + 1) % n) {
							continue;
						}
						if ((p1.second + 1) % n == f2.first) {
							return true;
						}
						for (auto& p2: palin_factors) {
							if (p2.first != (p1.second + 1) % n || 
								(p2.second + 1) % n != f2.first) {
								continue;
							}
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

static bool has_case_7_tiling(const word auto& w, 
	const vector<Factor>& onetwenty_factors) 
{
	// Looking for factorization A f_120(A) B f_120(B) C t_120(C)
	// [Case 7, IH07]
	size_t n = w.size();
	vector<set<Factor>> factor_starts(n);
	// cerr << "Case 7 factors" << endl;

	for (auto& f: onetwenty_factors) {
		// cerr << f.first << ", " << f.second << endl;
		factor_starts[f.first].insert(f);
	}

	for (const auto& A: onetwenty_factors) {
		size_t A_len = (n + A.second - A.first + 1) % n;
		for (const auto& B: factor_starts[(A.second + 1) % n]) {
			size_t B_len = (n + B.second - B.first + 1) % n;

			// One empty 120-drome.
			if (A_len + B_len == n) {
				return true;
			}

			// No empty 120-drome factors.
			for (const auto& C: factor_starts[(B.second + 1) % n]) {
				size_t C_len = (n + C.second - C.first + 1) % n;
				if (A_len + B_len + C_len == n) {
					return true;
				}
			}
		}
	}

	return false;
}

static bool has_case_8a_tiling(const word auto& w,
	const vector<Factor>& palin_factors, 
	const vector<Factor>& sixty_factors, 
	const vector<Factor>& onetwenty_factors) 
{
	// Looking for factorization A f_60(A) B f_120(B) C
	// where C is palindrome. [Case 8a, IH21]
	size_t n = w.size();
	vector<vector<Factor>> sixty_factor_starts(n);
	for (auto& f: sixty_factors) {
		sixty_factor_starts[f.first].push_back(f);
	}
	vector<vector<Factor>> onetwenty_factor_starts(n);
	for (auto& f: onetwenty_factors) {
		onetwenty_factor_starts[f.first].push_back(f);
	}

	// Factorizations with non-empty palindrome factor.
	for (auto& C: palin_factors) {
		size_t C_len = C.second - C.first + 1 + n * (C.second < C.first);
		// Empty 60-drome factor.
		for (auto& B: onetwenty_factor_starts[(C.second + 1)%n]) {
			size_t B_len = B.second - B.first + 1 + n * (B.second < B.first);
			if (B_len + C_len == n) {
				return true;
			}
		}

		// No empty 60-drome factor
		for (auto& A: sixty_factor_starts[(C.second + 1)%n]) {
			size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
			// Empty 120-drome factor.
			if (A_len + C_len == n) {
				return true;
			}

			// No empty 120-drome factor.
			for (auto& B: onetwenty_factor_starts[(A.second + 1)%n]) {
				int B_len = B.second - B.first + 1 + n * (B.second < B.first);
				if (A_len + B_len + C_len == n) {
					return true;
				}
			}
		}
	}

	// Factorizations with empty palindrome factor and non-empty 60-drome factor
	for (auto& A: sixty_factors) {
		size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
		for (auto& B: sixty_factor_starts[(A.second + 1)%n]) {
			size_t B_len = B.second - B.first + 1 + n * (B.second < B.first);
			if (A_len + B_len == n) {
				return true;
			}
		}
	}

	return false;
}

// TODO: it should be possible to avoid this function by re-running 8a on 
// a reflection of w.  But you'd have to be clever about modifying the
// pre-computed factors (or you'd have to recompute them...).
static bool has_case_8b_tiling(const word auto& w,
	const vector<Factor>& palin_factors, 
	const vector<Factor>& sixty_factors, 
	const vector<Factor>& onetwenty_factors) 
{
	// Looking for factorization A f_60(A) B C f_120(C)
	// where B is a palindrome. [Case 8b, IH21]
	size_t n = w.size();
	vector<vector<Factor>> sixty_factor_starts(n);
	for (auto& f: sixty_factors) {
		sixty_factor_starts[f.first].push_back(f);
	}
	vector<vector<Factor>> onetwenty_factor_starts(n);
	for (auto& f: onetwenty_factors) {
		onetwenty_factor_starts[f.first].push_back(f);
	}

	// Factorizations with non-empty palindrome factor.
	for (auto& B: palin_factors) {
		size_t B_len = B.second - B.first + 1 + n * (B.second < B.first);
		// Empty 120-drome factor.
		for (auto& A: sixty_factor_starts[(B.second + 1) % n]) {
			size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
			if (A_len + B_len == n) {
				return true;
			}
		}

		// No empty 120-drome factor
		for (auto& C: onetwenty_factor_starts[(B.second + 1) % n]) {
			size_t C_len = C.second - C.first + 1 + n * (C.second < C.first);
			// Empty 60-drome factor.
			if (B_len + C_len == n) {
				return true;
			}
			// No empty 60-drome factor.
			for (auto& A: sixty_factor_starts[(C.second + 1) % n]) {
				size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
				if (A_len + B_len + C_len == n) {
					return true;
				}
			}
		}
	}

	// Factorizations with empty palindrome factor and no empty 120-drome factor
	for (auto& C: onetwenty_factors) {
		size_t C_len = C.second - C.first + 1 + n * (C.second < C.first);
		for (auto& A: sixty_factor_starts[(C.second + 1) % n]) {
			size_t A_len = A.second - A.first + 1 + n * (A.second < A.first);
			if (A_len + C_len == n) {
				return true;
			}
		}
	}

	return false;
}

bool IsohedralChecker::tilesIsohedrally(
	const std::vector<point<int8_t>>& w, size_t order)
{
	vector<FactorPair> mirror_factor_pairs = 
		admissible_gapped_mirror_factor_pairs(w);
	vector<Factor> palin_factors = admissible_rotadrome_factors(w, 
		xform<int8_t> {1, 0, 0, 0, 1, 0}, true);

	if (has_half_turn_tiling(w, mirror_factor_pairs, palin_factors)) {
		return true;
	}

	vector<Factor> mirror_factors = admissible_mirror_factors(w);

	if (has_translation_tiling(w, mirror_factors)) {
		return true;
	}
  
  	if (order == 4) {
		// -90 degree rotation
		vector<Factor> ninety_factors = admissible_rotadrome_factors(
			w, xform<int8_t> {0, 1, 0, -1, 0, 0});

		if (has_quarter_turn_tiling(w, ninety_factors, palin_factors)) {
			return true;
		}
	}

	vector<Factor> glide_factors;

	if (order == 6) {
		glide_factors = admissible_glide_factors(w, hex_refls, 6);
	} else {
		glide_factors = admissible_glide_factors(w, square_refls, 4);
	}

	if (has_type_1_reflection_tiling(w, glide_factors, mirror_factor_pairs)) {
		return true;
	}

	if (has_type_1_half_turn_reflection_tiling(
		w, mirror_factor_pairs, palin_factors, glide_factors)) {
		return true;
	}

	if (has_type_2_reflection_tiling(w, mirror_factors, 
		order == 6 ? hex_refls : square_refls, order == 6 ? 6 : 4)) {
		return true;
	}

	if (has_type_2_half_turn_reflection_tiling(w, palin_factors,
		order == 6 ? hex_refls : square_refls, order == 6 ? 6 : 4)) {
		return true;
	}

	if (order == 6) {
		// Pass in negative rotation to support CCW boundary orderering.
		// Rotate by 60 degrees here to support a 120 degree joint between
		// the two halves of the drome.
		vector<Factor> onetwenty_factors = admissible_rotadrome_factors(
			w, xform<int8_t> {1, 1, 0, -1, 0, 0});
		// cerr << onetwenty_factors.size() << endl;

		if (has_case_7_tiling(w, onetwenty_factors)) {
			return true;
		}

		// -120 degree rotation
		vector<Factor> sixty_factors = admissible_rotadrome_factors(
			w, xform<int8_t> {0, 1, 0, -1, -1, 0});
		if (has_case_8a_tiling(
			w, palin_factors, sixty_factors, onetwenty_factors)) {
			return true;
		}
		if (has_case_8b_tiling(
			w, palin_factors, sixty_factors, onetwenty_factors)) {
			return true;
		}
	}

	return false;
}
