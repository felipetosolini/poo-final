#pragma once

#include <vector>
#include <optional>
#include <algorithm>

namespace utils {

/**
 * Busca el primer elemento en un vector que cumpla una condición.
 * Implementación genérica (template) reutilizable para cualquier tipo T.
 * 
 * @tparam T Tipo de elemento en el vector
 * @tparam Predicate Función o lambda que retorna bool
 * @param vec Vector a buscar
 * @param pred Predicado (condición)
 * @return std::optional<T> — elemento encontrado o vacío
 */
template<typename T, typename Predicate>
std::optional<T> findFirst(const std::vector<T>& vec, Predicate pred) {
    auto it = std::find_if(vec.begin(), vec.end(), pred);
    return it != vec.end() ? std::optional<T>(*it) : std::nullopt;
}

/**
 * Busca todos los elementos que cumplan una condición.
 * 
 * @tparam T Tipo de elemento
 * @tparam Predicate Función o lambda que retorna bool
 * @param vec Vector a buscar
 * @param pred Predicado (condición)
 * @return std::vector<T> Vector con todos los elementos encontrados
 */
template<typename T, typename Predicate>
std::vector<T> findAll(const std::vector<T>& vec, Predicate pred) {
    std::vector<T> result;
    std::copy_if(vec.begin(), vec.end(), std::back_inserter(result), pred);
    return result;
}

/**
 * Aplica una transformación a todos los elementos del vector.
 * 
 * @tparam T Tipo de entrada
 * @tparam U Tipo de salida
 * @tparam Transformer Función que transforma T → U
 * @param vec Vector de entrada
 * @param transformer Función de transformación
 * @return std::vector<U> Vector transformado
 */
template<typename T, typename U, typename Transformer>
std::vector<U> map(const std::vector<T>& vec, Transformer transformer) {
    std::vector<U> result;
    std::transform(vec.begin(), vec.end(), std::back_inserter(result), transformer);
    return result;
}

} // namespace utils
