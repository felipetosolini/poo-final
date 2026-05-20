from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from core.database import engine
from models import user, match  # importar para que SQLAlchemy registre los modelos
from core.database import Base
from routers import auth, matches, stats

Base.metadata.create_all(bind=engine)

app = FastAPI(title="Chess Insight AI", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(auth.router)
app.include_router(matches.router)
app.include_router(stats.router)


@app.get("/health")
def health():
    return {"status": "ok"}
