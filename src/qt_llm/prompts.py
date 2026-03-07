SYSTEM_DEFAULT = """You are a senior C++/Qt engineer.
Focus on Qt Widgets, signals/slots, threading, performance, and clean architecture.
Be concise and correct. When writing code, prefer modern C++ and Qt best practices.
If changing code, output the final code or a clear patch-style diff.
"""

def make_user_prompt(task: str, selection: str, extra: str = "") -> str:
    # Keep prompt structure stable for predictable outputs
    parts = [f"Task: {task.strip()}"]
    if extra.strip():
        parts.append("Extra context/instructions:")
        parts.append(extra.strip())
    parts.append("=== Input ===")
    parts.append(selection.rstrip())
    parts.append("=== End Input ===")
    return "\n".join(parts) + "\n"

def task_to_system(task: str) -> str:
    # You can specialize system prompt per task if you want later
    return SYSTEM_DEFAULT