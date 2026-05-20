from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from core.database import get_db
from core.security import decode_token
from services.statistics_repository import StatisticsRepository
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials

router = APIRouter(prefix="/stats", tags=["stats"])
bearer = HTTPBearer()


def get_current_user_id(credentials: HTTPAuthorizationCredentials = Depends(bearer)) -> int:
    try:
        payload = decode_token(credentials.credentials)
        return int(payload["sub"])
    except Exception:
        raise HTTPException(status_code=401, detail="Token inválido o expirado")


@router.get("/{user_id}")
def get_stats(
    user_id: int,
    current_user_id: int = Depends(get_current_user_id),
    db: Session = Depends(get_db),
):
    # Solo el propio usuario puede ver sus stats
    if user_id != current_user_id:
        raise HTTPException(status_code=403, detail="Acceso denegado")

    repo = StatisticsRepository(db)
    return {
        "user_id": user_id,
        "match_count": repo.get_match_count(user_id),
        "blunder_count": repo.get_blunder_count(user_id),
        **repo.get_accuracy_averages(user_id),
        "evolution": repo.get_accuracy_evolution(user_id),
    }
