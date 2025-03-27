import numpy as np

# Create a 1-D array
arr_1d = np.array([1, 2, 3, 4, 5])
arr_1d2 = np.array([1, 2, 3, 4, 5])
print(arr_1d + arr_1d2)
print(arr_1d * 3)
print(
    f"""iloczyn skalarny arr1 = {arr_1d} 
i arr2 = {arr_1d2} wynosi = {np.dot(arr_1d, arr_1d2)}
Sprawdzenie czy numpy nie oszukuje:
  {np.sum(arr_1d * arr_1d2)}"""
)

# Create a 2-D array
arr_2d = np.array([[1, 2, 3], [4, 5, 6], [4, 5, 6]])
arr_2d2 = np.array([[-1, -2, -3], [14, 15, 16], [4, 5, 6]])
print(
    f""" 
suma macierzy wynosi = 
{arr_2d + arr_2d2}
iloczyn macierzy po współrzędnych wynosi =
{arr_2d * arr_2d2}
iloczyn macierzy (standardowy)
{np.dot(arr_2d, arr_2d2)} lub
{np.matmul(arr_2d, arr_2d2)}
"""
)
v: np.ndarray = np.array([1, 2, 3, 4, 5])
print(
    f"""
Jak wybrać podzbiór macierzy, wektora,
v[0, 1] = {v[0]}, {v[1]}
v[0, 1] = {v[0:2]}
co drugi wyraz vectora
{v[::2]}
[odkąd = 0:dokąd = do końca:co ile= co jeden]
ostatni element v = {v[-1]}
dwa ostatnie elementy v {v[-2:]}
v od końca = {v[::-2][:2]}
"""
)
m: np.ndarray = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
print(
    f"""
m =
{m}
1 kolumna macierz m = 
{m[:, 0]}
1 i2 kolumna m = 
{m[:, :2]}
1 i 3 wiersz m = 
{m[::2, :]}
macierz m bez pierwszego wiersza i bez pierwszej kolumny = 
{m[1:, 1:]}
"""
)


def zero_rows(m):
    """
    tutaj zakładamy że pivot m[0,0] != 0 i za jego
    pomocą chcemy wyzerować pierwszą kolumnę macierzy m
    """
    if m.shape[0] < 2:
        return
    first_row = m[0, :]
    pivot = first_row[0]
    for row in m[1:, :]:
        row_pivot = row[0]
        row -= row_pivot / pivot * first_row


def swap_rows(m, i, j):
    m[[i, j]] = m[[j, i]]


def gaussian_elimination(m: np.ndarray):
    print(f"gaussina elimination m =\n{m}")
    if m.size == 0:
        return
    if m.shape[0] == 1:
        return
    pivot = m[0, 0]
    first_column = m[:, 0]
    should_swap = pivot == 0
    if should_swap:
        non_zero_indeces = np.nonzero(first_column)[0]
        if len(non_zero_indeces) == 0:
            gaussian_elimination(m[:, 1:])
            return
        first_non_zero_index = non_zero_indeces[0]
        swap_rows(m, 0, first_non_zero_index)
    zero_rows(m)
    return gaussian_elimination(m[1:, 1:])


m = m.astype(np.float64)
# m[0, 0] = 0.0
m[:, 1] = m[:, 0]
print(
    f"""
macierz m = 
{m}"""
)

gaussian_elimination(m)
print(
    f"""
zeschodkowana macierz m =
{m}\n
and det(m) = ????
"""
)
