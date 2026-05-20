#pragma once

#include <QString>
#include <memory>

namespace chess {

enum class PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None
};

enum class PieceColor {
    White,
    Black,
    None
};

// Clase abstracta — todo debe heredar de aquí
class Piece {
public:
    virtual ~Piece() = default;

    // Métodos virtuales puros
    virtual PieceType getType() const = 0;
    virtual QString getSymbol() const = 0;

    // Métodos concretos
    PieceColor getColor() const { return color; }
    void setColor(PieceColor c) { color = c; }

    virtual Piece* clone() const = 0;

protected:
    explicit Piece(PieceColor c) : color(c) {}

private:
    PieceColor color;
};

// Clases concretas heredando de Piece
class Pawn : public Piece {
public:
    explicit Pawn(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::Pawn; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "P" : "p"; }
    Piece* clone() const override { return new Pawn(*this); }
};

class Knight : public Piece {
public:
    explicit Knight(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::Knight; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "N" : "n"; }
    Piece* clone() const override { return new Knight(*this); }
};

class Bishop : public Piece {
public:
    explicit Bishop(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::Bishop; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "B" : "b"; }
    Piece* clone() const override { return new Bishop(*this); }
};

class Rook : public Piece {
public:
    explicit Rook(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::Rook; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "R" : "r"; }
    Piece* clone() const override { return new Rook(*this); }
};

class Queen : public Piece {
public:
    explicit Queen(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::Queen; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "Q" : "q"; }
    Piece* clone() const override { return new Queen(*this); }
};

class King : public Piece {
public:
    explicit King(PieceColor c) : Piece(c) {}
    PieceType getType() const override { return PieceType::King; }
    QString getSymbol() const override { return getColor() == PieceColor::White ? "K" : "k"; }
    Piece* clone() const override { return new King(*this); }
};

} // namespace chess
