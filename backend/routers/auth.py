from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from core.database import get_db
from core.security import create_access_token
from schemas.user import UserRegister, UserLogin, TokenResponse, UserOut
from services.user_repository import UserRepository

router = APIRouter(prefix="/auth", tags=["auth"])


@router.post("/register", response_model=TokenResponse, status_code=status.HTTP_201_CREATED)
def register(data: UserRegister, db: Session = Depends(get_db)):
    repo = UserRepository(db)
    if repo.get_by_username(data.username):
        raise HTTPException(status_code=400, detail="El nombre de usuario ya existe")
    if repo.get_by_email(data.email):
        raise HTTPException(status_code=400, detail="El email ya está registrado")
    user = repo.create(data.username, data.email, data.password)
    token = create_access_token({"sub": str(user.id), "username": user.username})
    return TokenResponse(access_token=token, user=UserOut.model_validate(user))


@router.post("/login", response_model=TokenResponse)
def login(data: UserLogin, db: Session = Depends(get_db)):
    repo = UserRepository(db)
    user = repo.authenticate(data.username, data.password)
    if not user:
        raise HTTPException(status_code=401, detail="Credenciales inválidas")
    token = create_access_token({"sub": str(user.id), "username": user.username})
    return TokenResponse(access_token=token, user=UserOut.model_validate(user))
