#ifndef UTIL_TINY_VECTOR_HPP
#define UTIL_TINY_VECTOR_HPP

namespace util {
	template <typename T, unsigned int CAPACITY>
	class TinyVector {
	private:
		T elements[CAPACITY];
		unsigned int s;

	public:
		TinyVector() : s(0) {}

		TinyVector(const TinyVector& v) : s(v.s) {
			for (unsigned int i = 0; i < s; ++i)
				elements[i] = v.elements[i];
		}

		TinyVector& operator=(const TinyVector &v) {
			s = v.s;
			for (unsigned int i = 0; i < s; ++i)
				elements[i] = v.elements[i];
			return *this;
		}

		inline T& operator[](std::size_t pos) {
			assert(pos >= 0);
			assert(pos < s);
			return elements[pos];
		}
		inline const T& operator[](std::size_t pos) const {
			assert(pos >= 0);
			assert(pos < s);
			return elements[pos];
		};

		inline unsigned int size() const {
			return s;
		}

		T const * begin() const {
			return &elements[0];
		}
		T const * end() const {
			return &elements[s];
		}

		T * begin() {
			return &elements[0];
		}
		T * end() {
			return &elements[s];
		}

		inline bool empty() const {
			return s == 0;
		}

		inline void clear() {
			s = 0;
		}
		inline void truncate(unsigned int size) {
			assert(size >= 0);
			assert(size <= s);
			s = size;
		}
		inline void pop_back() {
			assert(s > 0);
			--s;
		}

		inline void push_back(const T &element) {
			assert(s < CAPACITY);
			elements[s++] = element;
			if (s > CAPACITY) {
				int* die = NULL;
				std::cerr << "TinyVector over capacity! " << CAPACITY << std::endl;
				die[666] = 666;
			}
		}
	};
}

#endif // UTIL_TINY_VECTOR_HPP
