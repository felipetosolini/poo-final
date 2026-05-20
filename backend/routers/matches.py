from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from core.database import get_db
from core.security import decode_token
from schemas.match import MatchCreate, MatchOut
from services.match_repository import MatchRepository
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials

router = APIRouter(prefix="/matches", tags=["matches"])
bearer = HTTPBearer()


def get_current_user_id(credentials: HTTPAuthorizationCredentials = Depends(bearer)) -> int:
    try:
        payload = decode_token(credentials.credentials)
        return int(payload["sub"])
    except Exception:
        raise HTTPException(status_code=401, detail="Token inválido o expirado")


@router.get("/", response_model=list[MatchOut])
def list_matches(
    user_id: int = Depends(get_current_user_id),
    db: Session = Depends(get_db),
):
    return MatchRepository(db).get_all_by_user(user_id)


@router.post("/", response_model=MatchOut, status_code=status.HTTP_201_CREATED)
def create_match(
    data: MatchCreate,
    user_id: int = Depends(get_current_user_id),
    db: Session = Depends(get_db),
):
    return MatchRepository(db).create(user_id, data)
