import json
import requests
from typing import List, Dict, Any, Optional
from .config import Settings, parse_headers

class OpenAICompatClient:
    """
    Minimal OpenAI-compatible chat client:
      POST {base_url}/chat/completions
    Supports:
      - Local servers (Ollama /v1, vLLM /v1, LM Studio /v1, etc.)
      - OpenAI cloud (https://api.openai.com/v1)
    """

    def __init__(self, settings: Settings):
        self.s = settings

    def _headers(self) -> Dict[str, str]:
        h = {
            "Content-Type": "application/json",
        }
        # If api_key provided, use Bearer. (OpenAI requires)
        if self.s.api_key.strip():
            h["Authorization"] = f"Bearer {self.s.api_key.strip()}"
        # Extra user headers (optional)
        h.update(parse_headers(self.s.extra_headers))
        return h

    def chat_completions(
        self,
        messages: List[Dict[str, str]],
        model: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
    ) -> str:
        base = self.s.base_url.rstrip("/")
        # accept base_url either ".../v1" or ".../v1/"
        # endpoint is always /chat/completions relative to base
        url = f"{base}/chat/completions"

        payload: Dict[str, Any] = {
            "model": model or self.s.model,
            "messages": messages,
            "temperature": self.s.temperature if temperature is None else temperature,
        }

        # Some servers accept max_tokens; for others it’s ignored but harmless.
        if (max_tokens or self.s.max_tokens) > 0:
            payload["max_tokens"] = max_tokens if max_tokens is not None else self.s.max_tokens

        r = requests.post(url, headers=self._headers(), data=json.dumps(payload), timeout=self.s.timeout_s)

        # Better error message
        if r.status_code >= 400:
            try:
                j = r.json()
                raise RuntimeError(f"HTTP {r.status_code}: {j}")
            except Exception:
                raise RuntimeError(f"HTTP {r.status_code}: {r.text[:800]}")

        data = r.json()

        # OpenAI style: choices[0].message.content
        choices = data.get("choices", [])
        if not choices:
            return ""
        msg = choices[0].get("message", {}) or {}
        return (msg.get("content") or "").strip()